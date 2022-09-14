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

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <assert.h>
#include <string.h>

// this should be enough
static char buf[65536] = {};
static int buf_index = 0;
static char code_buf[65536 + 128] = {}; // a little larger than `buf`
static char *code_format =
"#include <stdio.h>\n"
"int main() { "
"  unsigned result = %s; "
"  printf(\"%%u\", result); "
"  return 0; "
"}";

int choose(int n){ return rand() % n;}

void addspace(){
    int seed = choose(100);
    int n;
    if(seed > 95) n = 3;
    else if(seed > 85) n = 2;
    else if(seed > 70) n = 1;
    else n = 0;
    for(int i = 0; i < n; i++) buf[buf_index++] = ' ';
}

void gen(char c){
	buf[buf_index++] = c;
	addspace();
}


unsigned gen_num(int limit) {
    int len;
    unsigned num;
    if(limit == -1) num = 1;
    else if(limit == -2) num = 0;
    else num = (unsigned)rand() % 256;
    do{ len = sprintf(buf+buf_index, "%u", num);
    }while(len >= limit);
    buf_index += len;
    addspace();
    return num;
}

int gen_rand_op() {
    int c = choose(4);
    switch(c){
        case 0: buf[buf_index++] = '+'; break;
        case 1: buf[buf_index++] = '-'; break;
        case 2: buf[buf_index++] = '*'; break;
        default: buf[buf_index++] = '/'; break;
    }
    addspace();
    return c;
}

unsigned gen_rand_expr(int limit, int depth) {
    if(depth == 0) return gen_num(limit);
    unsigned ret, ret1, ret2;
    int op;
    switch (choose(3)) {
        case 0:
            ret = gen_num(limit);  break;
        case 1:
            gen('('); 
            ret = gen_rand_expr(limit-2, depth-1);
            gen(')');  break;
        default:
            ret1 = gen_rand_expr(limit, depth-1);
            op = gen_rand_op(); 
            int temp = buf_index;
            do{
            	buf_index = temp;
            	ret2 = gen_rand_expr(limit-1-temp, depth-1);
            }while((op == 1 && ret1 < ret2) || (op == 3 && ret2 == 0));
            if(op == 2){
	      unsigned t = ret1 * ret2;
	      if(t != 0 && (t / ret2 != ret1 || t / ret1 != ret2)){
	        buf_index = temp;
		gen_num(-1);
	      }
	    }
	    else if(op == 0){
	      unsigned t = ret1 + ret2;
	      if(t < ret1 && t < ret2){
	        buf_index = temp;
		gen_num(-2);
	      }
	    }
            switch (op) {
                case 0: ret = ret1 + ret2; break;
                case 1: ret = ret1 - ret2; break;
                case 2: ret = ret1 * ret2; break;
                default: ret = ret1 / ret2; break;
            }
    }
    return ret;
}

int main(int argc, char *argv[]) {
  int seed = time(0);
  srand(seed);
  int loop = 1;
  if (argc > 1) {
    sscanf(argv[1], "%d", &loop);
  }
  int i;
  for (i = 0; i < loop; i ++) {
    gen_rand_expr(65500, 30);
    buf[buf_index] = '\0';
    buf_index = 0;
    
    sprintf(code_buf, code_format, buf);

    FILE *fp = fopen("/tmp/.code.c", "w");
    assert(fp != NULL);
    fputs(code_buf, fp);
    fclose(fp);

    int ret = system("gcc /tmp/.code.c -o /tmp/.expr");
    if (ret != 0) continue;

    fp = popen("/tmp/.expr", "r");
    assert(fp != NULL);

    int result;
    int fs = fscanf(fp, "%d", &result);
    if(fs < 0) printf("fuck");
    pclose(fp);

    printf("%u %s\n", result, buf);
  }
  return 0;
}
