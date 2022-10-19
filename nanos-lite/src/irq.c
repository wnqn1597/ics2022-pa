#include <common.h>

void do_syscall(Context *c);
Context* schedule(Context *prev);

static Context* do_event(Event e, Context* c) {
  switch (e.event) {
		case EVENT_YIELD: c->mepc += 4; Log("YIELD"); break;
		case EVENT_IRQ_TIMER: break;
		case EVENT_SYSCALL: c->mepc += 4; break;
    default: panic("Unhandled event ID = %d", e.event);
  }
	do_syscall(c);
  return c;
}

void init_irq(void) {
  Log("Initializing interrupt/exception handler...");
  cte_init(do_event);
}
