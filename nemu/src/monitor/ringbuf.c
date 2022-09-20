#include <ringbuf.h>

#define RB_LEN 8
#define MRB_LEN 128

static RBN RBN_pool[RB_LEN] = {};
static RBN *head = NULL;
static bool init = false;

static MRBN MRBN_pool[MRB_LEN] = {};
static MRBN *mhead = NULL;
static bool minit = false;

void init_pool() {
	if(init) return;
    for(int i = 0; i < RB_LEN; i++) RBN_pool[i].next = (i == RB_LEN - 1) ? &RBN_pool[0] : &RBN_pool[i+1];
    head = RBN_pool;
    init = true;
}

void init_mpool() {
	if(minit) return;
    for(int i = 0; i < MRB_LEN; i++) MRBN_pool[i].next = (i == MRB_LEN - 1) ? &MRBN_pool[0] : &MRBN_pool[i+1];
    mhead = MRBN_pool;
    minit = true;
}

void insert(uint32_t instr, uint32_t pc) {
    head->instr = instr;
    head->pc = pc;
    head = head->next;
}

void minsert(int type, uint32_t addr, uint32_t content) {
    mhead->type = type;
    mhead->addr = addr;
    mhead->content = content;
    mhead = mhead->next;
}

void display_pool() {
    RBN *now = head;
    while(now->next != head){
        if(now->instr != 0) printf("\t0x%08x:\t\t%08x\n", now->pc, now->instr);
        now = now->next;
    }
    printf("-->\t0x%08x:\t\t%08x\n", now->pc, now->instr);
}

void display_mpool() {
    MRBN *now = mhead;
    printf("TYPE\tADDR\t\tCONTENT\n");
    do {
        if(now->type != 0) printf("%s\t0x%08x\t%08x\n", (now->type == 1) ? "read" : "write", now->addr, now->content);
        now = now->next;
    } while(now != mhead);
}
