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
#include <cpu/cpu.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <memory/host.h>
#include <memory/paddr.h>
#include "sdb.h"

static int is_batch_mode = false;

void init_regex();
void isa_reg_display();
uint8_t* guest_to_host(paddr_t paddr);
static inline word_t host_read(void *addr, int len);
static inline bool in_pmem(paddr_t addr);

#ifdef CONFIG_WATCHPOINT
void init_wp_pool();
WP* new_up();
void free_wp(WP *wp);
void watchpoint_display();
#endif

/* We use the `readline' library to provide more flexibility to read from stdin. */
static char* rl_gets() {
  static char *line_read = NULL;

  if (line_read) {
    free(line_read);
    line_read = NULL;
  }

  line_read = readline("(nemu) ");

  if (line_read && *line_read) {
    add_history(line_read);
  }

  return line_read;
}

static int cmd_c(char *args) {
  cpu_exec(-1);
  return 0;
}


static int cmd_q(char *args) {
  nemu_state.state = NEMU_QUIT;
  return -1;
}

static int cmd_si(char *args){
  char *arg = strtok(NULL, "");
  if(arg == NULL){
    cpu_exec(1);
  }else{
    int line = atoi(arg);
    cpu_exec(line);
  }
  return 0;
}

static int cmd_info(char *args) {
  char *arg = strtok(NULL, " ");
  if(arg == NULL){
    printf("Unknown command\n");
  }else if(strcmp(arg, "r") == 0){
    //printf("Print GPRs\n");
    isa_reg_display();
  }else if(strcmp(arg, "w") == 0){
    //printf("Print watching point\n");
#ifdef CONFIG_WATCHPOINT
    watchpoint_display();
#else
	printf("Watchpoint disabled.\n");
#endif
  }else{
    printf("Unknown command '%s'\n", arg);
  }
  return 0;
}

static int cmd_x(char *args){
  char *arg1 = strtok(NULL, " ");
  if(arg1 == NULL) return 0;
  int bytes = atoi(arg1);
  char *base_addr = strtok(NULL, " ");
  if(base_addr == NULL) return 0;
  uint32_t paddr = strtol(base_addr, (char**)NULL, 16);
  for(int i = 0; i < bytes; i++){
    if(!in_pmem(paddr)){
      printf("Addr: %x is out of bound.\n", paddr);
      break;
    }
    uint32_t content = host_read(guest_to_host(paddr), 4);
    printf("M[%x]: %x\n", paddr, content);
    paddr += 4;
  }	
  return 0;
}

static int cmd_p(char *args){
  bool success = true;
  char *e = strtok(NULL, " ");
  if(e == NULL){
    printf("No expression given.\n");
    return 0;
  }
  printf("expr:%s\n", e);
  word_t result = expr(e, &success);
  if(success) printf("Result:\ns: %d\nu: %u\nh: %x\n", result, result, result);
  else printf("Calculation failed.\n");
  return 0;
}

static int cmd_w(char *args){
#ifdef CONFIG_WATCHPOINT
  char *e = strtok(NULL, " ");
  if(e == NULL){
    printf("No watchpoint address given.\n");
    return 0;
  }
  bool success = true;
  word_t value = expr(e, &success);
  if(!success){
    printf("Caculation failed.\n");
    return 0;
  }
  WP *nwp = new_up();
  strcpy(nwp->expression, e);
  nwp->pre_val = value;
#else
  printf("Watchpoint disabled.\n");
#endif
  return 0;
}

static int cmd_d(char *args){
#ifdef CONFIG_WATCHPOINT
  char *arg = strtok(NULL, " ");
  if(arg == NULL){
    printf("No watchpoint number given.\n");
    return 0;
  }
  int NO = atoi(arg);
  WP *nwp = get_head();
  while(nwp != NULL){
    if(nwp->NO == NO) break;
    nwp = nwp->next;
  }
  if(nwp == NULL){
    printf("Cannot find watchpoint NO %d\n", NO);
    return 0;
  }
  free_wp(nwp);
#else
  printf("Watchpoint disabled.\n");
#endif
  return 0;
}

static int cmd_help(char *args);

static struct {
  const char *name;
  const char *description;
  int (*handler) (char *);
} cmd_table [] = {
  { "help", "Display information about all supported commands", cmd_help },
  { "c", "Continue the execution of the program", cmd_c },
  { "q", "Exit NEMU", cmd_q },
  { "si", "Execute the program step by step", cmd_si },
  { "info", "Print the infomation of the program", cmd_info },
  { "x", "Scan the memory", cmd_x },
  { "p", "Calculate the expression", cmd_p},
  { "w", "Create a new watchpoint", cmd_w},
  { "d", "Delete a watchpoint", cmd_d}
};

#define NR_CMD ARRLEN(cmd_table)

static int cmd_help(char *args) {
  /* extract the first argument */
  char *arg = strtok(NULL, " ");
  int i;

  if (arg == NULL) {
    /* no argument given */
    for (i = 0; i < NR_CMD; i ++) {
      printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
    }
  }
  else {
    for (i = 0; i < NR_CMD; i ++) {
      if (strcmp(arg, cmd_table[i].name) == 0) {
        printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
        return 0;
      }
    }
    printf("Unknown command '%s'\n", arg);
  }
  return 0;
}

void sdb_set_batch_mode() {
  is_batch_mode = true;
}

void sdb_mainloop() {
  if (is_batch_mode) {
    cmd_c(NULL);
    return;
  }

  for (char *str; (str = rl_gets()) != NULL; ) {
    char *str_end = str + strlen(str);

    /* extract the first token as the command */
    char *cmd = strtok(str, " ");
    if (cmd == NULL) { continue; }

    /* treat the remaining string as the arguments,
     * which may need further parsing
     */
    char *args = cmd + strlen(cmd) + 1;
    if (args >= str_end) {
      args = NULL;
    }

#ifdef CONFIG_DEVICE
    extern void sdl_clear_event_queue();
    sdl_clear_event_queue();
#endif

    int i;
    for (i = 0; i < NR_CMD; i ++) {
      if (strcmp(cmd, cmd_table[i].name) == 0) {
        if (cmd_table[i].handler(args) < 0) { return; }
        break;
      }
    }

    if (i == NR_CMD) { printf("Unknown command '%s'\n", cmd); }
  }
}

void init_sdb() {
  /* Compile the regular expressions. */
  init_regex();
#ifdef CONFIG_WATCHPOINT
  /* Initialize the watchpoint pool. */
  init_wp_pool();
#endif
}
