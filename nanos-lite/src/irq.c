#include <common.h>

static Context* do_event(Event e, Context* c) {
  switch (e.event) {
		case EVENT_YIELD: c->mepc += 4; Log("YIELD"); break;
		case EVENT_IRQ_TIMER: break;
		case EVENT_SYSCALL: printf("sys\n");c->mepc += 4; break;
    default: panic("Unhandled event ID = %d", e.event);
  }

  return c;
}

void init_irq(void) {
  Log("Initializing interrupt/exception handler...");
  cte_init(do_event);
}
