#include <unistd.h>
#include <stdio.h>
#include <NDL.h>

int main(){
  NDL_Init(0);
  uint32_t pre = NDL_GetTicks();
  while(1) {
    uint32_t now = NDL_GetTicks();
    if(now - pre >= 500) {
      printf("skr~\n");
      pre = now;
    }
  }
  return 0;
}
