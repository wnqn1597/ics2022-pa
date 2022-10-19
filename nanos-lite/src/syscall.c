#include <common.h>
#include "syscall.h"

void sys_exit(Context *c) {
	halt(c->GPRx);
}

void sys_yield(Context *c) {
	yield();
	c->GPRx = 0;
}

void sys_write() {

}

void do_syscall(Context *c) {
  uintptr_t a[4];
  a[0] = c->GPR1;
	a[1] = c->GPR2;
	a[2] = c->GPR3;
	a[3] = c->GPR4;
  switch (a[0]) {
		case SYS_exit: sys_exit(c); break;
		case SYS_yield: sys_yield(c); break;
		//case SYS_open: sys_open(c, (char*)a[1], a[2], a[3]); break;
		//case SYS_read: sys_read(c, a[1], (void*)a[2], a[3]); break;
		//case SYS_write: sys_write(c, a[1], (void*)a[2], a[3]); break;
		//case SYS_close: sys_close(c, a[1]); break;
		//case SYS_lseek: sys_lseek(c, a[1], a[2], a[3]); break;
		//case SYS_brk: sys_brk(c, a[1]); break;
		//case SYS_execve: sys_execve(c, (char*)a[1], (char**)a[2], (char**)a[3]); break;
		//case SYS_gettimeofday: sys_gettimeofday(c); break;
    default: panic("Unhandled syscall ID = %d", a[0]);
  }
}
