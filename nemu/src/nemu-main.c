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

#include <common.h>
//#include <monitor/sdb/sdb.h>

void init_monitor(int, char *[]);
void am_init_monitor();
void engine_start();
int is_exit_status_bad();

word_t expr(char *e, bool *success);

int main(int argc, char *argv[]) {
  /* Initialize the monitor. */
#ifdef CONFIG_TARGET_AM
  am_init_monitor();
#else
  init_monitor(argc, argv);
#endif

#ifdef TEST
  FILE *fp = fopen("/home/ubuntu/ics2022/nemu/tools/gen-expr/input", "r");
  assert(fp);
  char buf[1024];
  uint32_t u;
  bool success = true;
  while(fgets(buf, 1024, fp) != NULL){
  	buf[strlen(buf)-1] = '\0';
    int i = 0;
    while(buf[i]!= ' ') i++;
    u = atoi(buf);
    printf("u=%u,expr=%s,", u, buf+i+1);
    word_t result = expr(buf+i+1, &success);
    if(!success) assert(0);
	printf("result=%u\n", result);
    assert(result == u);
  }
#else
  
  /* Start engine. */
  engine_start();
#endif
  return is_exit_status_bad();
}
