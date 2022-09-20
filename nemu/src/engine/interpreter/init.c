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

#include <cpu/cpu.h>
#include <ringbuf.h>

void sdb_mainloop();

void engine_start() {
#ifdef CONFIG_ITRACE_RINGBUF
  init_pool();
#endif
#ifdef CONFIG_MTRACE
  init_mpool();
#endif

#ifdef CONFIG_TARGET_AM
  while(1);
  cpu_exec(-1);
#else
  /* Receive commands from user. */
  sdb_mainloop();
#endif
}
