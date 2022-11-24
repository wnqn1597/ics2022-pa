#include <am.h>
#include <nemu.h>
#include <klib.h>

typedef union {
	struct {
		uint32_t offs : 12;
		uint32_t vpn0 : 10;
		uint32_t vpn1 : 10;
	};
	uint32_t val;
} Vaddr;

typedef union {
	struct {
		uint32_t offs : 12;
		uint32_t ppn  : 20;
	};
	uint32_t val;
} Paddr;

typedef union {
	struct {
		uint32_t v   : 1;
		uint32_t r   : 1;
		uint32_t w   : 1;
		uint32_t x   : 1;
		uint32_t u   : 1;
		uint32_t g   : 1;
		uint32_t a   : 1;
		uint32_t d   : 1;
		uint32_t rsw : 2;
		uint32_t ppn : 22;
	};
	uint32_t val;
} PageTableEntry;

static AddrSpace kas = {};
static void* (*pgalloc_usr)(int) = NULL;
static void (*pgfree_usr)(void*) = NULL;
static int vme_enable = 0;

static Area segments[] = {      // Kernel memory mappings
  NEMU_PADDR_SPACE
};

#define USER_SPACE RANGE(0x40000000, 0x80000000)

static inline void set_satp(void *pdir) {
  uintptr_t mode = 1ul << (__riscv_xlen - 1);
  asm volatile("csrw satp, %0" : : "r"(mode | ((uintptr_t)pdir >> 12)));
}

static inline uintptr_t get_satp() {
  uintptr_t satp;
  asm volatile("csrr %0, satp" : "=r"(satp));
  return satp << 12;
}

bool vme_init(void* (*pgalloc_f)(int), void (*pgfree_f)(void*)) {
  pgalloc_usr = pgalloc_f;
  pgfree_usr = pgfree_f;

  kas.ptr = pgalloc_f(PGSIZE);

  int i;
  for (i = 0; i < LENGTH(segments); i ++) {
    void *va = segments[i].start;
    for (; va < segments[i].end; va += PGSIZE) {
      map(&kas, va, va, 0);
    }
  }

  set_satp(kas.ptr);
  vme_enable = 1;

  return true;
}

void protect(AddrSpace *as) {
  PTE *updir = (PTE*)(pgalloc_usr(PGSIZE));
  as->ptr = updir;
  as->area = USER_SPACE;
  as->pgsize = PGSIZE;
  // map kernel space
  memcpy(updir, kas.ptr, PGSIZE);
}

void unprotect(AddrSpace *as) {
}

void __am_get_cur_as(Context *c) {
  c->pdir = (vme_enable ? (void *)get_satp() : NULL);
}

void __am_switch(Context *c) {
  if (vme_enable && c->pdir != NULL) {
    set_satp(c->pdir);
  }
}

void map(AddrSpace *as, void *va, void *pa, int prot) {
	//printf("map from v=%p to p=%p\n", va, pa);
	Vaddr vaddr = {.val = (uintptr_t)va};
	Paddr paddr = {.val = (uintptr_t)pa};
	if(vaddr.offs != paddr.offs) assert(0);

	uint32_t *pdirBase = (uint32_t*)as->ptr;
	PageTableEntry pdirPTE = {.val = *(pdirBase + vaddr.vpn1)};
	if(pdirPTE.v == 0){
		uint32_t newPTabBase = (uint32_t)(uintptr_t)pgalloc_usr(PGSIZE);
		pdirPTE.ppn = (newPTabBase >> 12); // newPTabBase % (1 << 12) == 0
		pdirPTE.v = 1;
		*(pdirBase + vaddr.vpn1) = pdirPTE.val;
	}

	uint32_t *ptabBase = (uint32_t*)(pdirPTE.ppn << 12);
	PageTableEntry ptabPTE = {.val = *(ptabBase + vaddr.vpn0)};
	if(ptabPTE.v == 0){
		ptabPTE.ppn = paddr.ppn;
		ptabPTE.v = 1;
		*(ptabBase + vaddr.vpn0) = ptabPTE.val;
	}else assert(0);
}

Context *ucontext(AddrSpace *as, Area kstack, void *entry) {
	// Page
	uint32_t *pdir = (uint32_t*)(kstack.end - 1 * 4);
	*pdir = (uintptr_t)as->ptr; // set page directory

	// Stack Exchange
	//uint32_t *sp = (uint32_t*)(kstack.end - 34 * 4);
	//*sp = (uintptr_t)USER_SPACE.end; // set sp = 0x80000000
	//uint32_t *np = (uint32_t*)(kstack.end - 36 * 4); // use the space of register x0
	//*np = 0; // USER_CONTEXT_TAG

	uint32_t *mepc_ptr 		= (uint32_t*)(kstack.end - 2 * 4);
	uint32_t *mstatus_ptr = (uint32_t*)(kstack.end - 3 * 4);
	*mstatus_ptr = 0x1880;
	*mepc_ptr = (uintptr_t)entry;
	return (Context*)(kstack.end - 36 * 4); // the head of context
}
