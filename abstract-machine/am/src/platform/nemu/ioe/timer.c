#include <am.h>
#include <nemu.h>
#include <klib.h>
#include <riscv/riscv.h>

static uint64_t boot_us = 0;

void __am_timer_init() {
  uint32_t secl = inl(RTC_ADDR);
  uint64_t sech = inl(RTC_ADDR+4);
  boot_us = (sech << 32) | secl;
  printf("boot_time=%d\n", boot_us);
}

void __am_timer_uptime(AM_TIMER_UPTIME_T *uptime) {
  printf("timer_uptime\n");
  uint32_t secl = inl(RTC_ADDR);
  printf("secl=%d, ", secl);
  uint64_t sech = inl(RTC_ADDR+4);
  printf("sech=%d, ", sech);
  uint64_t current_us = (sech << 32) | secl;
  printf("ret=%d\n", current_us);
  uptime->us = current_us - boot_us;
}

void __am_timer_rtc(AM_TIMER_RTC_T *rtc) {
  rtc->second = 0;
  rtc->minute = 0;
  rtc->hour   = 0;
  rtc->day    = 0;
  rtc->month  = 0;
  rtc->year   = 1900;
}
