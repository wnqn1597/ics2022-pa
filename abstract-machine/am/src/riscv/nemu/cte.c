#include <am.h>
#include <riscv/riscv.h>
#include <klib.h>

static Context* (*user_handler)(Event, Context*) = NULL;

void __am_get_cur_as(Context *c);
void __am_switch(Context *c);

// extern char _end;

Context* __am_irq_handle(Context *c) {
	// Page
	__am_get_cur_as(c);
  if (user_handler) {
    Event ev = {0};
    switch (c->mcause) {
			case 0x80000003: ev.event = EVENT_YIELD; break;
			case 0x80000007: ev.event = EVENT_IRQ_TIMER; break;
			case 0x0000000b: ev.event = EVENT_SYSCALL; break;
      default: ev.event = EVENT_ERROR; break;
    }

    c = user_handler(ev, c);
    assert(c != NULL);
  }
	// Page
	if(c->pdir != NULL) __am_switch(c);
  return c;
}

extern void __am_asm_trap(void);

bool cte_init(Context*(*handler)(Event, Context*)) {
  // initialize exception entry
  asm volatile("csrw mtvec, %0" : : "r"(__am_asm_trap));

	// Stack Exchange
	// asm volatile("csrw mscratch, %0" : : "r"(&_end));

  // register event handler
  user_handler = handler;

  return true;
}

Context *kcontext(Area kstack, void (*entry)(uint32_t), uint32_t arg) {
	// Page
	uint32_t *pdir = (uint32_t*)(kstack.end - 1 * 4);
	*pdir = 0;

	// Stack Exchange
	//uint32_t *sp = (uint32_t*)(kstack.end - 34 * 4);
	//*sp = (uintptr_t)kstack.end;
	//uint32_t *np = (uint32_t*)(kstack.end - 36 * 4); // use the space of register x0
	//*np = 1; // KERNEL_CONTEXT_TAG

	uint32_t *mepc_ptr 		= (uint32_t*)(kstack.end - 2 * 4);
	uint32_t *mstatus_ptr = (uint32_t*)(kstack.end - 3 * 4);
	uint32_t *arg_ptr 		= (uint32_t*)(kstack.end - 26 * 4); // a0
	*mstatus_ptr = 0x1880;
	*mepc_ptr = (uintptr_t)entry;
	*arg_ptr = arg;
	return (Context*)(kstack.end - 36 * 4); // the head of context
}

void yield() {
  asm volatile("li a7, -1; ecall");
}

bool ienabled() {
  return false;
}

void iset(bool enable) {
}
