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
#include <memory/host.h>
/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <regex.h>

enum {
  TK_NOTYPE = 256, TK_EQ, TK_NEQ, TK_AND, TK_REGNAME,
  TK_DEC, TK_HEX, TK_LPA, TK_RPA, TK_NEG, TK_DEREF, TK_PC, TK_REG

};

static struct rule {
  const char *regex;
  int token_type;
} rules[] = {

  /* TODO: Add more rules.
   * Pay attention to the precedence level of different rules.
   */
  {" +", TK_NOTYPE},    // spaces
  {"\\+", '+'},         // plus
  {"-", '-'},		// minus
  {"\\*", '*'},		// multiply
  {"/", '/'},		// divide
  {"==", TK_EQ},        // equal
  {"!=", TK_NEQ},	// not equal
  {"&&", TK_AND},	// logic and
  {"([a-z]{1,2}[0-9]{0,2})|(\\$0)", TK_REGNAME},	// register name
  {"0x[0-9a-f]+", TK_HEX},	// hexadecimal
  {"[0-9]+", TK_DEC},	// decimal
  {"\\(", TK_LPA},	// left_parenthese
  {"\\)", TK_RPA},	// right_parenthese
  {"_", TK_NEG},	// negative
  {"~", TK_DEREF},	// dereference
  {"\\$pc", TK_PC},	// visit pc
  {"\\$", TK_REG},	// visit register
};

#define NR_REGEX ARRLEN(rules)

static regex_t re[NR_REGEX] = {};

/* Rules are used for many times.
 * Therefore we compile them only once before any usage.
 */
void init_regex() {
  int i;
  char error_msg[128];
  int ret;

  for (i = 0; i < NR_REGEX; i ++) {
    ret = regcomp(&re[i], rules[i].regex, REG_EXTENDED);
    if (ret != 0) {
      regerror(ret, &re[i], error_msg, 128);
      panic("regex compilation failed: %s\n%s", error_msg, rules[i].regex);
    }
  }
}

#define TOKENS_LEN 64
#define TOKEN_STRLEN 32
typedef struct token {
  int type;
  char str[TOKEN_STRLEN];
} Token;

static Token tokens[TOKENS_LEN] __attribute__((used)) = {};
static int nr_token __attribute__((used))  = 0;

static void refresh(){
  for(int i = 0; i < TOKENS_LEN; i++){
    tokens[i].type = 0;
    memset(tokens[i].str, 0, TOKEN_STRLEN);
  }
  nr_token = 0;
}

static inline bool judge(char *c){
  return *c == '+' || *c == '-' || *c == '*' || *c == '/' || *c == '(' || *c == '_';
}

static bool make_token(char *s) {
  char e[strlen(s)+1];
  strcpy(e, s);
  char *_e = e;
  if(*_e == '-') *_e = '_';
  if(*_e == '*') *_e = '~';
  //_e++;
  while(*_e != '\0'){
    if(judge(_e)){
      if(*(_e+1) == '-') *(_e+1) = '_';
      else if(*(_e+1) == '*') *(_e+1) = '~';
    }
    _e++;
  }

  int position = 0;
  int i;
  regmatch_t pmatch;

  nr_token = 0;

  while (e[position] != '\0') {
    /* Try all rules one by one. */
    for (i = 0; i < NR_REGEX; i ++) {
      if (regexec(&re[i], e + position, 1, &pmatch, 0) == 0 && pmatch.rm_so == 0) {
        char *substr_start = e + position;
        int substr_len = pmatch.rm_eo;

        //Log("match rules[%d] = \"%s\" at position %d with len %d: %.*s",
        //    i, rules[i].regex, position, substr_len, substr_len, substr_start);

        position += substr_len;

        /* TODO: Now a new token is recognized with rules[i]. Add codes
         * to record the token in the array `tokens'. For certain types
         * of tokens, some extra actions should be performed.
         */

        switch (rules[i].token_type) {
	  	  case TK_NOTYPE: break;
		  case TK_REGNAME:
	  	  case TK_DEC:
	  	  case TK_HEX: strncpy(tokens[nr_token].str, substr_start, substr_len);
          default: tokens[nr_token++].type = rules[i].token_type;
        }

        break;
      }
    }

    if (i == NR_REGEX) {
      printf("no match at position %d\n%s\n%*.s^\n", position, e, position, "");
      return false;
    }
  }

  return true;
}

bool check_parentheses(int p, int q){
  if(tokens[p].type != TK_LPA || tokens[q].type != TK_RPA)return false;
  int count = 0;
  bool ret = true;
  for(int i = p; i <= q; i++){
    if(tokens[i].type == TK_LPA) count++;
    else if(tokens[i].type == TK_RPA) count--;
    if(count < 0) Log("Bad expression.");
    else if(count == 0 && i != q) ret = false;
  }
  return ret;
}

int get_main_token(int p, int q){
  int count = 0;
  bool chosen_mul = false, chosen_plus = false, chosen_eq = false;
  int ret = -1;
  for(int i = q; i >= p; i--){
    if(tokens[i].type == TK_RPA) count++;
    else if(tokens[i].type == TK_LPA) count--;
    if(count > 0) continue;
    if(!chosen_mul && !chosen_plus && !chosen_eq && (tokens[i].type == '*' || tokens[i].type == '/')){
      ret = i;
      chosen_mul = true;
    }else if(!chosen_plus && !chosen_eq && (tokens[i].type == '+' || tokens[i].type == '-')){
      ret = i;
      chosen_plus = true;
    }else if(!chosen_eq && (tokens[i].type == TK_EQ || tokens[i].type == TK_NEQ)){
      ret = i;
      chosen_eq = true;
    }else if(tokens[i].type == TK_AND) return i;
  }
  return ret;
}

word_t eval(int p, int q, bool *success){
  if(p > q){
    Log("Bad expression.");
    assert(0);
  }else if(p == q){
    if(tokens[p].type == TK_HEX) return (word_t)strtol(tokens[p].str, (char**)NULL, 16);
    else if(tokens[p].type == TK_DEC) return (word_t)atoi(tokens[p].str);
    else if(tokens[p].type == TK_REGNAME) return regname_to_index(tokens[p].str);
    else if(tokens[p].type == TK_PC) return cpu.pc;
    else { *success = false;return 0;}
  }else if(check_parentheses(p, q) == true){
    return eval(p+1, q-1, success);
  }else{
    int index = get_main_token(p, q);
    if(index == -1){
      if(tokens[p].type == TK_NEG) return -1 * eval(p+1, q, success);
      else if(tokens[p].type == TK_DEREF){
        return host_read(guest_to_host(eval(p+1, q, success)), 4);     
      }else if(tokens[p].type == TK_REG){
        word_t index = eval(p+1, q, success);
	if(index >= 32||index < 0){ *success = false;return 0;}
        word_t ret = isa_reg_str2val(regs[index], success);
	if(success)return ret;
	else { *success = false;return 0;}
      }
    }
    word_t val1 = eval(p, index-1, success);
    word_t val2 = eval(index+1, q, success);

    switch(tokens[index].type){
      case '+': return val1 + val2;
      case '-': return val1 - val2;
      case '*': return val1 * val2;
      case '/': return val1 / val2;
      case TK_EQ: return val1 == val2;
      case TK_NEQ: return val1 != val2;
      case TK_AND: return val1 && val2;
      default: *success = false; return 0;
    }
  }
}


word_t expr(char *e, bool *success) {
  init_regex();
  if (!make_token(e)) {
    *success = false;
    return 0;
  }
  word_t ret = eval(0, nr_token-1, success);
  //printf("Expression result: %d\n", ret);
  refresh();
  return ret;
}
