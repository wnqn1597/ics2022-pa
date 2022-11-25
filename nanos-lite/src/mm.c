#include <memory.h>
#include <proc.h>

static void *pf = NULL;
extern PCB *current;

void* new_page(size_t nr_page) {
	void *old_pf = pf;
	pf += nr_page * PGSIZE;
	memset(old_pf, 0, pf - old_pf);
  return old_pf;
}

#ifdef HAS_VME
static void* pg_alloc(int n) {
  return new_page(n / PGSIZE);
}
#endif

void free_page(void *p) {
  panic("not implement yet");
}

/* The brk() system call handler. */
int mm_brk(uintptr_t brk) {
	uint32_t cmb = current->max_brk;
  if(cmb < brk) {
		if((cmb & 0xfff) != 0) cmb = (cmb & ~0xfff) + PGSIZE;
		while(cmb < brk) {
			map(&current->as, (void*)cmb, new_page(1), 0);
			cmb += PGSIZE;
		}
		current->max_brk = brk;
	}
	return 0;
}

void init_mm() {
  pf = (void *)ROUNDUP(heap.start, PGSIZE);
  Log("free physical pages starting from %p", pf);

#ifdef HAS_VME
  vme_init(pg_alloc, free_page);
#endif
}
