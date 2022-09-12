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

#include "sdb.h"

#define NR_WP 32

static WP wp_pool[NR_WP] = {};
static WP *head = NULL, *free_ = NULL, *headtail = NULL, *freetail = NULL;

void init_wp_pool() {
  int i;
  for (i = 0; i < NR_WP; i ++) {
    wp_pool[i].NO = i;
    wp_pool[i].next = (i == NR_WP - 1 ? NULL : &wp_pool[i + 1]);
  }

  head = NULL;
  headtail = NULL;
  free_ = wp_pool;
  freetail = &wp_pool[NR_WP-1];
}

WP* get_head(){
  return head;
}

WP* new_up(){
  if(free_ == NULL){
    printf("Insufficient watchpoint.\n");
    return NULL;
  }
  if(head == NULL || headtail == NULL){
    head = free_;
    free_->NO = 1;
  }else{
    headtail->next = free_;
    free_->NO = headtail->NO + 1;
  }
  headtail = free_;
  free_ = free_->next;
  headtail->next = NULL;
  if(free_ == NULL) freetail = NULL;
  return headtail;
}

void free_wp(WP *wp){
  if(head == wp){
    head = wp->next;
    if(head == NULL) headtail = NULL;
  }else{
    WP *now = head;
    while(now != NULL){
      if(now->next == wp)break;
      now = now->next;
    }
    if(now == NULL){
      printf("Not using watchpoint\n");
      return;
    }
    now->next = wp->next;
  }
  wp->next = NULL;
  if(free_ == NULL || freetail == NULL) free_ = wp;
  else freetail->next = wp;
  freetail = wp;
}

void watchpoint_display(){
  if(head == NULL){
    printf("No created watchpoint.\n");
    return;
  }
  printf("No\tType\tValue\t\tExpression\n");
  WP *now = head;
  while(now != NULL){
    bool success;
    word_t value = expr(now->expression, &success);
    if(!success) printf("%d\t%d\t%s\t%s\n", now->NO, now->type, "NULL", now->expression);
    else printf("%d\t%d\t%x\t%s\n", now->NO, now->type, value, now->expression);
    now = now->next;
  }
}
