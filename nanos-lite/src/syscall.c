#include <common.h>
#include "syscall.h"
#include <fs.h>
#include <proc.h>

void* get_finfo(int index, int property);

int fs_open(const char *pathname, int flags, int mode);
int fs_write(int fd, const void *buf, size_t len);
int fs_read(int fd, void *buf, size_t len);
int fs_close(int fd);
int fs_lseek(int fd, size_t offset, int whence);
int mm_brk(uintptr_t brk);

void naive_uload(PCB *pcb, const char *filename);
//void context_uload(PCB *this_pcb, const char* filename, char *const argv[], char *const envp[]);
//PCB* get_pcb(int index);

void sys_exit(Context *c) {
	halt(0);
	c->GPRx = 1;
}

void sys_yield(Context *c) {
	yield();
	c->GPRx = 0;
}

void sys_brk(Context *c, intptr_t addr) {
  //printf("CALL BRK\n");
	//c->GPRx = mm_brk((uintptr_t)addr);
	c->GPRx = 0;
}

void sys_open(Context *c, const char *pathname, int flags, int mode) {
  //printf("CALL OPEN\n");
  c->GPRx = fs_open(pathname, flags, mode);
}

void sys_write(Context *c, int fd, void *buf, size_t count) {
  //printf("CALL WRITE\n");
  WriteFn wfunc = get_finfo(fd, 4);
  if(wfunc != NULL) c->GPRx = wfunc(buf, 0, count);
  else c->GPRx = fs_write(fd, buf, count);
}

void sys_read(Context *c, int fd, void *buf, size_t count) {
  //printf("CALL READ\n");
  ReadFn rfunc = get_finfo(fd, 3);
  if(rfunc != NULL) c->GPRx = rfunc(buf, 0, count);
  else c->GPRx = fs_read(fd, buf, count);
}

void sys_close(Context *c, int fd) {
  //printf("CALL CLOSE\n");
  c->GPRx = fs_close(fd);
}

void sys_lseek(Context *c, int fd, size_t offset, int whence) {
  //printf("CALL LSEEK\n");
  c->GPRx = fs_lseek(fd, offset, whence);
}

void sys_gettimeofday(Context *c) {
  uint64_t usec = _gettimeofday();
  c->GPRx = usec;
  //c->GPR1 = sec >> 32;
}

//void sys_execve(Context *c, char *filename, char **exec_argv, char **envp) {
//  printf("sys_execve\n");
//  PCB *new_pcb = (current == get_pcb(0) ? get_pcb(1) : get_pcb(0));
//  context_uload(new_pcb, filename, exec_argv, envp);
//	yield();
//  c->GPRx = (uintptr_t)c;
//}

void do_syscall(Context *c) {
  uintptr_t a[4];
  a[0] = c->GPR1;
	a[1] = c->GPR2;
	a[2] = c->GPR3;
	a[3] = c->GPR4;
  switch (a[0]) {
		case SYS_exit: sys_exit(c); break;
		case SYS_yield: sys_yield(c); break;
		case SYS_open: sys_open(c, (char*)a[1], a[2], a[3]); break;
		case SYS_read: sys_read(c, a[1], (void*)a[2], a[3]); break;
		case SYS_write: sys_write(c, a[1], (void*)a[2], a[3]); break;
		case SYS_close: sys_close(c, a[1]); break;
		case SYS_lseek: sys_lseek(c, a[1], a[2], a[3]); break;
		case SYS_brk: sys_brk(c, a[1]); break;
		//case SYS_execve: sys_execve(c, (char*)a[1], (char**)a[2], (char**)a[3]); break;
		case SYS_gettimeofday: sys_gettimeofday(c); break;
    default: panic("Unhandled syscall ID = %d", a[0]);
  }
}
