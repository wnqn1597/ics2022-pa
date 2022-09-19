#include <am.h>
#include <klib.h>
#include <klib-macros.h>
#include <stdarg.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

int     BIT64   = 0x1,
        BIT128  = 0x2,
        SPACE   = 0x4,
        SPECIAL = 0x8,
        ZEROPAD = 0x10,
        SMALL   = 0x20,
        SIGN    = 0x40;

int is_digit(char ch){
    return ch >= '0' && ch <= '9';
}

int skip_atoi(const char **s){
    int i = 0;
    while(is_digit(**s)) i = i * 10 + *((*s)++) - '0';
    return i;
}

char * number(char * str, int64_t num, int base, int size, int precision, int type)
{
    char c,sign,tmp[36];
    const char *digits="0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    int i;

    if (type&SMALL) digits="0123456789abcdefghijklmnopqrstuvwxyz";
    if (base<2 || base>36) return 0;
    c = (type & ZEROPAD) ? '0' : ' ' ;
    if (!(type&BIT64)) num = (int32_t)num;
    if ((type&SIGN) && num<0) {
        sign='-';
        num = -num;
    }
    if (sign) size--;
    if (type&SPECIAL){
        if (base==16) size -= 2;
        else if (base==8) size--;
    }
    i=0;
    if (num==0) tmp[i++]='0';
    else {
        while (num!=0) {
            tmp[i++]=digits[num % base];
            num /= base;
        }
    }

    if (i>precision) precision=i;
    size -= precision;
    if (!(type&(ZEROPAD)))
        while(size-->0)
            *str++ = ' ';
    if (sign)
        *str++ = sign;
    if (type&SPECIAL){
        if (base==8) *str++ = '0';
        else if (base==16) {
            *str++ = '0';
            *str++ = digits[33];
        }
    }
    while(size-->0)
        *str++ = c;
    while(i<precision--)
        *str++ = '0';
    while(i-->0)
        *str++ = tmp[i];
    while(size-->0)
        *str++ = ' ';
    return str;
}

int vsprintf(char *buf, const char *fmt, va_list args) {
    int len;
    int i;
    char * str;
    char *s;
    int *ip;

    int flags;

    int field_width;
    int precision;

    for (str=buf ; *fmt ; fmt++) {
        if (*fmt != '%') { // 非%就复制
            *str++ = *fmt;
            continue;
        }


        flags = 0;
        repeat:
        fmt++; // 到%后一个
        switch (*fmt) {
            case ' ': flags |= SPACE; goto repeat;
            case '#': flags |= SPECIAL; goto repeat;
            case '0': flags |= ZEROPAD; goto repeat;
        }


        field_width = -1;
        if (is_digit(*fmt))
            field_width = skip_atoi(&fmt);
        else if (*fmt == '*') {
            field_width = va_arg(args, int);
        }


        precision = -1;
        if (*fmt == '.') {
            ++fmt;
            if (is_digit(*fmt)) precision = skip_atoi(&fmt);
            else if (*fmt == '*') precision = va_arg(args, int);
            if (precision < 0) precision = 0;
        }


        if (*fmt == 'l') {
            fmt++;
        }
        if(*fmt == 'l') {
            flags |= BIT64;
            fmt++;
        }

        switch (*fmt) {
            case 'c':
                while (--field_width > 0) *str++ = ' ';
                *str++ = (char)va_arg(args, int);
                while (--field_width > 0) *str++ = ' ';
                break;

            case 's':
                s = va_arg(args, char*);
                len = (int)strlen(s);
                if (precision < 0)
                    precision = len;
                else if (len > precision)
                    len = precision;

                while (len < field_width--)
                    *str++ = ' ';
                for (i = 0; i < len; ++i)
                    *str++ = *s++;
                while (len < field_width--)
                    *str++ = ' ';
                break;

            case 'o':
                if(flags & BIT64)  str = number(str, va_arg(args, uint64_t), 8, field_width, precision, flags);
                else               str = number(str, va_arg(args, uint32_t), 8, field_width, precision, flags);
                break;

            case 'p':
                if (field_width == -1) {
                    field_width = 8;
                    flags |= ZEROPAD;
                }
                str = number(str, (uint64_t)(uintptr_t) va_arg(args, void *), 16, field_width, precision, flags);
                break;

            case 'x':
                flags |= SMALL;
            case 'X':
                if(flags & BIT64)   str = number(str, va_arg(args, uint64_t), 16, field_width, precision, flags);
                else                str = number(str, va_arg(args, uint32_t), 16, field_width, precision, flags);
                break;

            case 'd':
            case 'i':
                flags |= SIGN;
            case 'u':
                if(flags & BIT64)   str = number(str, va_arg(args, uint64_t), 10, field_width, precision, flags);
                else                str = number(str, va_arg(args, uint32_t), 10, field_width, precision, flags);
                break;

            case 'n':
                ip = va_arg(args, int *);
                *ip = (str - buf);
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
    int n;

    va_start(args, fmt);
    n = vsprintf(buf, fmt, args);
    va_end(args);

    //char *b;
    //for(b = buf; *b != '\0'; b++) putch(*b);
    //putch(*b);
    return n;
}

int printf(const char *fmt, ...) {
  //char *buf;
  char buf[300];
  va_list args;
  int n;
  va_start(args, fmt);
  n = vsprintf(buf, fmt, args);
  va_end(args);

  //for(; *buf != '\0'; buf++) putch(*buf);
  //putch(*buf);
  int i;
  for(i = 0; i < 100 && buf[i] != '\0'; i++) putch(buf[i]);
  putch(buf[i]);
  return n;
}

int snprintf(char *out, size_t n, const char *fmt, ...) {
  panic("Not implemented");
}

int vsnprintf(char *out, size_t n, const char *fmt, va_list ap) {
  panic("Not implemented");
}

#endif
