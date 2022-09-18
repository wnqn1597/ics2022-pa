#include <am.h>
#include <nemu.h>
#include <klib.h>

static uint64_t boot_us = 0;

uint64_t _gettimeofday() {
  uint32_t secl = inl(RTC_ADDR);
  uint64_t sech = inl(RTC_ADDR+4);
  return (sech << 32) | secl;
}

void __am_timer_init() {
  boot_us = _gettimeofday();
  printf("init=%d\n", boot_us);
}

void __am_timer_uptime(AM_TIMER_UPTIME_T *uptime) {
  uptime->us = _gettimeofday() - boot_us;
  printf("uptime=%d\n", uptime->us);
}

void __am_timer_rtc(AM_TIMER_RTC_T *rtc) {
  rtc->second = 0;
  rtc->minute = 0;
  rtc->hour   = 0;
  rtc->day    = 0;
  rtc->month  = 0;
  rtc->year   = 1900;
}
