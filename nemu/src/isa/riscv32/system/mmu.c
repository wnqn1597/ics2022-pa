/***************************************************************************************
* Copyright (c) 2014-2022 Zihao Yu, Nanjing University
*
* NEMU is licensed under Mulan PSL v2.
* You can use this software according to the terms and conditions of the Mulan PSL v2.
* You may obtain a copy of Mulan PSL v2 at:
*          http://license.coscl.org.cn/MulanPSL2
*
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
* EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
* MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
*
* See the Mulan PSL v2 for more details.
***************************************************************************************/

#include <isa.h>
#include <memory/paddr.h>
#include <memory/vaddr.h>

typedef union {
	struct {
		uint32_t offs : 12;
		uint32_t vpn0 : 10;
		uint32_t vpn1 : 10;
	};
	uint32_t val;
} Vaddr;

int isa_mmu_check(vaddr_t vaddr, int len, int type) {
	uint32_t satp_val = getcsr(0x180);
	return (satp_val >> 31) == 1 ? MMU_TRANSLATE : MMU_DIRECT;
}

paddr_t isa_mmu_translate(vaddr_t vaddr, int len, int type) {
	Vaddr v = {.val = vaddr};
	uint32_t *pdirBase = (uint32_t*)(uintptr_t)(getcsr(0x180) << 12);
	uint32_t pdirPTE = paddr_read((uintptr_t)(pdirBase + v.vpn1), 4);
	if((pdirPTE & 0x1) == 0) { Log("vaddr=%08x\n", vaddr);assert(0);}

	uint32_t *ptabBase = (uint32_t*)(uintptr_t)((pdirPTE & ~0x3ff) << 2);
	uint32_t ptabPTE = paddr_read((uintptr_t)(ptabBase + v.vpn0), 4);
	if((ptabPTE & 0x1) == 0) { Log("vaddr=%08x\n", vaddr);assert(0);}
	return ((ptabPTE & ~0x3ff) << 2) | v.offs;
}
