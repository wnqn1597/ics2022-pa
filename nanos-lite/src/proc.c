#include <proc.h>

#define MAX_NR_PROC 4

static PCB pcb[MAX_NR_PROC] __attribute__((used)) = {};
static PCB pcb_boot = {};
PCB *current = NULL;

void naive_uload(PCB *pcb, const char *filename);
uintptr_t loader(PCB *pcb, const char *filename);

PCB* get_pcb(int index) {
	if(index < 0 || index >= MAX_NR_PROC) return NULL;
	return &pcb[index];
}

static void nanos_set_satp(void *pdir) {
	uintptr_t mode = 1ul << (__riscv_xlen - 1);
	asm volatile("csrw satp, %0" : : "r"(mode | ((uintptr_t)pdir >> 12)));
}

static uint32_t len(char *const arr[]) {
    if(arr == NULL) return 0;
    uint32_t ret;
    for(ret = 0;; ret++) {
        if(arr[ret] == NULL) break;
    }
    return ret;
}

static void* set_mainargs(AddrSpace *as, char *const argv[], char *const envp[]){
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

void map_ustack(AddrSpace *as) {
	void *npage;
	for(int i = 1; i <= 8; i++) {
		npage = new_page(1);
		map(as, as->area.end - i * PGSIZE, npage, 0);
	}
}

void context_kload(PCB *this_pcb, void (*entry)(uint32_t), uint32_t arg) {
	this_pcb->as.area.start = (void*)this_pcb;
	this_pcb->as.area.end = (void*)((uint8_t*)this_pcb + 8 * PGSIZE);

	this_pcb->cp = kcontext(this_pcb->as.area, entry, arg);
	//this_pcb->cp->GPRx = (uintptr_t)this_pcb->as.area.end; // with this line, the parameter `arg' doesn't work.
}

void context_uload(PCB *this_pcb, const char *filename, char* const argv[], char* const envp[]) {
	protect(&this_pcb->as);
	nanos_set_satp(this_pcb->as.ptr);
	
	void *entry = (void*)loader(this_pcb, filename);
	Area kstack = {.start = (void*)this_pcb, .end = (void*)this_pcb + 8 * PGSIZE};
	
	map_ustack(&this_pcb->as);
	this_pcb->cp = ucontext(&this_pcb->as, kstack, entry);
	
	// void *upage_start = new_page(8);
	// AddrSpace as = {.area.start = upage_start, .area.end = upage_start + 8 * PGSIZE};
	void *argc_ptr = set_mainargs(&this_pcb->as, argv, envp);
	//this_pcb->cp->GPRx = (uintptr_t)heap.end; // heap.end = 0x88000000
	this_pcb->cp->GPRx = (uintptr_t)argc_ptr;
}

void init_proc() {

	char *argv[] = {"/bin/nterm", NULL};

	//context_kload(&pcb[0], hello_fun, 1);
	//context_kload(&pcb[1], hello_fun, 2);
	context_uload(&pcb[0], "/bin/hello", NULL, NULL);
	context_uload(&pcb[1], "/bin/nterm", argv, NULL);

	switch_boot_pcb();
  // load program here
	// naive_uload(NULL, "/bin/menu");
  Log("Initializing processes...");
}

static int counter = 0;

Context* schedule(Context *prev) {
  current->cp = prev; // record the addr of context
	
	counter ++;

	if(current == &pcb[0]) current = &pcb[1];
	else if(counter % 100 == 0){
		counter = 0;
		current = &pcb[0];
	}

	//current = current == &pcb[0] ? &pcb[1] : &pcb[0];

	return current->cp;
}
