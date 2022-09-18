#include <klib.h>
#include <klib-macros.h>
#include <stdint.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

size_t strlen(const char *s) {
  size_t ret;
  for(ret = 0; *(s+ret) != '\0'; ret++);
  return ret;
}

char *strcpy(char *dst, const char *src) {
  int i;
  for(i = 0; *(src+i) != '\0'; i++){
    *(dst+i) = *(src+i);
  }
  *(dst+i) = *(src+i);
  return dst;
}

char *strncpy(char *dst, const char *src, size_t n) {
  int i;
  for(i = 0; *(src+i) != '\0' && i < n; i++){
    *(dst+i) = *(src+i);
  }
  *(dst+i) = *(src+i);
  if(i == n){
  	*(dst+i) = '\0';
  }
  return dst;
}

char *strcat(char *dst, const char *src) {
  int bias;
  for(bias = 0; *(dst+bias) != '\0'; bias++);
  int i;
  for(i = 0; *(src+i) != '\0'; i++) *(dst+bias+i) = *(src+i);
  *(dst+bias+i) = *(src+i);
  return dst;
}

int strcmp(const char *s1, const char *s2) {
  int i;
  for(i = 0; *(s1+i) != '\0' && *(s2+i) != '\0'; i++){
    if(*(s1+i) < *(s2+i)) return -1;
    else if(*(s1+i) > *(s2+i)) return 1;
  }
  if(*(s1+i) == '\0' && *(s2+i) == '\0') return 0;
  else if(*(s1+i) == '\0') return -1;
  else return 1;
}

int strncmp(const char *s1, const char *s2, size_t n) {
  int i;
  for(i = 0; *(s1+i) != '\0' && *(s2+i) != '\0' && i < n; i++){
    if(*(s1+i) < *(s2+i)) return -1;
    else if(*(s1+i) > *(s2+i)) return 1;
  }
  if((*(s1+i) == '\0' && *(s2+i) == '\0') || i == n) return 0;
  else if(*(s1+i) == '\0') return -1;
  else return 1;
}

void *memset(void *s, int c, size_t n) {
  for(int i = 0; i < n; i++) *(char*)(s+i) = c;
  return s;
}

void *memmove(void *dst, const void *src, size_t n) {
  if(dst > src && dst - src < n){
  	for(int i = n-1; i >= 0; i--) *(char*)(dst+i) = *(char*)(src+i);
  }else{
    for(int i = 0; i < n; i++) *(char*)(dst+i) = *(char*)(src+i);
  }
  return dst;
}

void *memcpy(void *out, const void *in, size_t n) {
  for(int i = 0; i < n; i++) *(char*)(out+i) = *(char*)(in+i);
  return out;
}

int memcmp(const void *s1, const void *s2, size_t n) {
  for(int i = 0; i < n; i++){
    if(*(char*)(s1+i) < *(char*)(s2+i)) return -1;
    else if(*(char*)(s1+i) > *(char*)(s2+i)) return 1;
  }
  return 0;
}

#endif
