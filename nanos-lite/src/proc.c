#include <proc.h>

#define MAX_NR_PROC 4

static PCB pcb[MAX_NR_PROC] __attribute__((used)) = {};
static PCB pcb_boot = {};
PCB *current = NULL;

void naive_uload(PCB *pcb, const char *filename);
uintptr_t loader(PCB *pcb, const char *filename);

void switch_boot_pcb() {
  current = &pcb_boot;
}

void hello_fun(uint32_t arg) {
  int j = 1;
  while (1) {
    Log("Hello World from Nanos-lite with arg '%d' for the %dth time!", arg, j);
    j ++;
    yield();
  }
}

void context_kload(PCB *this_pcb, void (*entry)(uint32_t), uint32_t arg) {
	this_pcb->as.area.start = (void*)this_pcb;
	this_pcb->as.area.end = (void*)((uint8_t*)this_pcb + 8 * PGSIZE);

	this_pcb->cp = kcontext(this_pcb->as.area, entry, arg);
	//this_pcb->cp->GPRx = (uintptr_t)this_pcb->as.area.end; // with this line, the parameter `arg' doesn't work.
}

void context_uload(PCB *this_pcb, const char *filename, char* const argv[], char* const envp[]) {
	void *entry = (void*)loader(this_pcb, filename);
	Area kstack = {.end = (void*)this_pcb + 8 * PGSIZE};
	this_pcb->cp = ucontext(NULL, kstack, entry);
	this_pcb->cp->GPRx = (uintptr_t)heap.end; // heap.end = 0x88000000
}

void init_proc() {
  Log("Initializing processes...");

	context_kload(&pcb[0], hello_fun, 1);
	context_uload(&pcb[1], "/bin/pal", NULL, NULL);

	switch_boot_pcb();
  // load program here
	// naive_uload(NULL, "/bin/menu");
}

Context* schedule(Context *prev) {
  current->cp = prev; // record the addr of context
	
	current = &pcb[0];

	return current->cp;
}
