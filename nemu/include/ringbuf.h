#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>

typedef struct ringbufnode {
    uint32_t instr;
    uint32_t pc;
    struct ringbufnode *next;
} RBN;

typedef struct memringbufnode {
    int type; // 0: undefined 1: read 2: write
    uint32_t addr;
    uint32_t content;
    struct memringbufnode *next;
} MRBN;

void init_pool();
void insert(uint32_t instr, uint32_t pc);
void display_pool();

void init_mpool();
void minsert(int type, uint32_t addr, uint32_t content);
void display_mpool();
