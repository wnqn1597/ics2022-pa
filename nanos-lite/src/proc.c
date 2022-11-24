#include <proc.h>

#define MAX_NR_PROC 4

static PCB pcb[MAX_NR_PROC] __attribute__((used)) = {};
static PCB pcb_boot = {};
PCB *current = NULL;

void naive_uload(PCB *pcb, const char *filename);
uintptr_t loader(PCB *pcb, const char *filename);

static uint32_t len(char *const arr[]) {
    if(arr == NULL) return 0;
    uint32_t ret;
    for(ret = 0;; ret++) {
        if(arr[ret] == NULL) break;
    }
    return ret;
}

static void* set_mainargs(AddrSpace *as, char *const argv[], char *const envp[]){
		putch('\n');
    uint32_t argc = len(argv); uint32_t envc = len(envp);
    uint32_t pe[envc+1]; uint32_t pa[argc+1];

    void *end = as->area.end;
    uint32_t l, i;
    for(i = 0; i < envc; i++) {
        l = strlen(envp[i]) + 1;
        memcpy(end - l, (void*)envp[i], l);
        end -= l;
        pe[i] = (uintptr_t)(char*)end;
    }
    pe[i] = 0;
		end --;
    for(i = 0; i < argc; i++) {
        l = strlen(argv[i]) + 1;
        memcpy(end - l, (void*)argv[i], l);
        end -= l;
        pa[i] = (uintptr_t)(char*)end;
    }
    pa[i] = 0;
    memcpy(end-4*(envc+1), pe, 4*(envc+1));
    end -= 4*(envc+1);
    memcpy(end-4*(argc+1), pa, 4*(argc+1));
    end -= 4*(argc+1);
    end -= 4;
    *(uint32_t*)end = argc;
    return end;
}

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
	//void *upage_start = new_page(8);
	AddrSpace as = {.area.end = heap.end};
	void *argc_ptr = set_mainargs(&as, argv, envp);
	//this_pcb->cp->GPRx = (uintptr_t)heap.end; // heap.end = 0x88000000
	this_pcb->cp->GPRx = (uintptr_t)argc_ptr;
}

void init_proc() {
  Log("Initializing processes...");

	char *argv[] = {"skip"};

	context_kload(&pcb[0], hello_fun, 1);
	context_uload(&pcb[1], "/bin/pal", argv, NULL);

	switch_boot_pcb();
  // load program here
	// naive_uload(NULL, "/bin/menu");
}

Context* schedule(Context *prev) {
  current->cp = prev; // record the addr of context
	
	current = current == &pcb[0] ? &pcb[1] : &pcb[0];

	return current->cp;
}
