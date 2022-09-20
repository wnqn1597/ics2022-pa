#include <am.h>
#include <klib.h>
#include <klib-macros.h>
#include <stdarg.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

#define BUFFER_LEN 256

int     BIT64   = 0x1,
        SPECIAL = 0x2,
        ZEROPAD = 0x4,
        SMALL   = 0x8,
        SIGN    = 0x10;

int is_digit(char ch){ return ch >= '0' && ch <= '9'; }

int skip_atoi(const char **s){
    int i = 0;
    while(is_digit(**s)) i = i * 10 + *((*s)++) - '0';
    return i;
}

char* number(char * str, uint64_t num, int base, int size, int type) {
    if(num == 48){
    putch('@');
    }
	
    uint32_t u32 = (uint32_t)num;
    int32_t i32 = (int32_t)num;
    uint64_t u64 = (uint64_t)num;
    int64_t i64 = (int64_t)num;

    char c, sign = 0, tmp[36];
    const char *digits="0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    if (type&SMALL) digits="0123456789abcdefghijklmnopqrstuvwxyz";

    if (base < 2 || base > 36) return 0;
    c = (type & ZEROPAD) ? '0' : ' ' ;

    if (type & SIGN) {
        if(type & BIT64 && i64 < 0) {
            sign='-';
            i64 = -i64;
        }
        else if(i32 < 0) {
            sign='-';
            i32 = -i32;
        }
    }

    if (sign) size--;
    if (type&SPECIAL) {
        if (base==16) size -= 2;
        else if (base==8) size--;
    }

    int i = 0;
    if (num == 0) tmp[i++]='0';
    else {
        if(type & SIGN){
            if(type & BIT64){
                while (i64 != 0) {
                    tmp[i++]=digits[i64 % base];
                    i64 /= base;
                    size--;
                }
            }else{
                while (i32 != 0) {
                    tmp[i++]=digits[i32 % base];
                    i32 /= base;
                    size--;
                }
            }
        }else{
            if(type & BIT64){
                while (u64 != 0) {
                    tmp[i++]=digits[u64 % base];
                    u64 /= base;
                    size--;
                }
            }else{
                while (u32 != 0) {
                    tmp[i++]=digits[u32 % base];
                    u32 /= base;
                    size--;
                }
            }
        }
    }

    if (!(type & ZEROPAD)){
        while(size-- > 0) *str++ = ' ';
    }
    if (sign) *str++ = sign;
    if (type&SPECIAL) {
        if (base==8) *str++ = '0';
        else if (base==16) {
            *str++ = '0';
            *str++ = digits[33];
        }
    }
    while(size-- > 0) *str++ = c;
    while(i-- > 0) *str++ = tmp[i];
    return str;
}

int vsprintf(char *buf, const char *fmt, va_list args) {
    int len, i, flags, field_width;
    char * str, *s;

    for (str=buf ; *fmt ; fmt++) {
        if (*fmt != '%') {
            *str++ = *fmt;
            continue;
        }

        flags = 0;
        repeat: fmt++;
        switch (*fmt) {
            case '#': flags |= SPECIAL; goto repeat;
            case '0': flags |= ZEROPAD; goto repeat;
        }

        field_width = -1;
        if (is_digit(*fmt)) field_width = skip_atoi(&fmt);

        if (*fmt == 'l') fmt++;
        if(*fmt == 'l') {
            flags |= BIT64;
            fmt++;
        }

        switch (*fmt) {
            case 'c':
                while (--field_width > 0) *str++ = ' ';
                *str++ = (char)va_arg(args, int);
                break;

            case 's':
                s = va_arg(args, char*);
                len = (int)strlen(s);
                while (field_width-- > len) *str++ = ' ';
                for (i = 0; i < len; i++) *str++ = *s++;
                break;

            case 'o':
                str = number(str, va_arg(args, uint64_t), 8, field_width, flags);
                break;

            case 'p':
                if (field_width == -1) {
                    field_width = 16;
                    flags |= ZEROPAD;
                }
                str = number(str, (uint64_t)(uintptr_t)va_arg(args, void *), 16, field_width, flags);
                break;

            case 'x':
                flags |= SMALL;
            case 'X':
                str = number(str, va_arg(args, uint64_t), 16, field_width, flags);
                break;

            case 'd':
            case 'i':
                flags |= SIGN;
            case 'u':
                str = number(str, va_arg(args, uint64_t), 10, field_width, flags);
                break;

            default:
                if (*fmt != '%') *str++ = '%';
                if (*fmt) *str++ = *fmt;
                else --fmt;
                break;
        }
    }
    *str = '\0';
    return str-buf;
}

int sprintf(char *buf, const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    int n = vsprintf(buf, fmt, args);
    va_end(args);
    return n;
}

int printf(const char *fmt, ...) {
  	char buf[BUFFER_LEN];
  	va_list args;
  	va_start(args, fmt);
  	int n = vsprintf(buf, fmt, args);
  	va_end(args);
  	putstr(buf);
  	return n;
}

int snprintf(char *buf, size_t size, const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    int n = vsprintf(buf, fmt, args);
    va_end(args);
    if(n >= size) *(buf+size-1) = '\0';
    return n;
}

int vsnprintf(char *buf, size_t size, const char *fmt, va_list args) {
  	int n = vsprintf(buf, fmt, args);
  	if(n >= size) *(buf+size-1) = '\0';
  	return n;
}

#endif
