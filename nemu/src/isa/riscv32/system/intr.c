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

CSR csr_reg = {.mstatus.val = 0x1800};

uint32_t getcsr(uint32_t code) {
  switch(code) {
    case 0x180: return csr_reg.satp;
    case 0x300: return csr_reg.mstatus.val;
    case 0x305: return csr_reg.mtvec;
	case 0x340: return csr_reg.mscratch;
    case 0x341: return csr_reg.mepc;
    case 0x342: return csr_reg.mcause;
    default: printf("Unknown csr code %x\n", code);assert(0);
  }
}

void setcsr(uint32_t code, word_t value) {
  switch(code) {
    case 0x180: csr_reg.satp = value;break;
    case 0x300: csr_reg.mstatus.val = value;break;
    case 0x305: csr_reg.mtvec = value;break;
	case 0x340: csr_reg.mscratch = value;break;
    case 0x341: csr_reg.mepc = value;break;
    case 0x342: csr_reg.mcause = value;break;
    default: printf("Unknown csr code %x\n", code);assert(0);
  }
}

word_t isa_raise_intr(word_t NO, vaddr_t epc) {
  /* TODO: Trigger an interrupt/exception with ``NO''.
   * Then return the address of the interrupt/exception vector.
   */
  // mstatus
  csr_reg.mstatus.MPIE = csr_reg.mstatus.MIE;
  csr_reg.mstatus.MIE = 0;
	
  csr_reg.mcause = NO;
  csr_reg.mepc = epc;
  return csr_reg.mtvec;
  //return 0;
}

word_t isa_out_intr() {
  // mstatus
  csr_reg.mstatus.MIE = csr_reg.mstatus.MPIE;
  csr_reg.mstatus.MPIE = 1;
  return csr_reg.mepc;
}

word_t isa_query_intr() {
  return INTR_EMPTY;
}
