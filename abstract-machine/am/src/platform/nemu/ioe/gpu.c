#include <am.h>
#include <nemu.h>
#include <klib.h>
#define SYNC_ADDR (VGACTL_ADDR + 4)

void __am_gpu_init() {
//  int i;
//  int w = 400;
//  int h = 300;
//  uint32_t *fb = (uint32_t*)(uintptr_t)FB_ADDR;
//  for(i = 0; i < w*h; i++) fb[i] = i;
//  outl(SYNC_ADDR, 1);
}

uint32_t get_height() {
  return inl(VGACTL_ADDR) & 0xffff;
}

uint32_t get_width() {
  return (inl(VGACTL_ADDR) & 0xffff0000) >> 16;
}

void __am_gpu_config(AM_GPU_CONFIG_T *cfg) {
  uint32_t wh = inl(VGACTL_ADDR);
  uint32_t height = wh & 0xffff; // 300
  uint32_t width = (wh & 0xffff0000) >> 16; // 400
  *cfg = (AM_GPU_CONFIG_T) {
    .present = true, .has_accel = false,
    .width = width, .height = height,
    .vmemsz = 0
  };
}

uint32_t* get_fb() {
  return (uint32_t*)(uintptr_t)FB_ADDR;
}

void do_sync() {
  outl(SYNC_ADDR, 1);
}

void __am_gpu_fbdraw(AM_GPU_FBDRAW_T *ctl) {
  if (ctl->sync) {
    uint32_t *fb = (uint32_t*)(uintptr_t)FB_ADDR;
    uint32_t bias = ctl->y * 400 + ctl->x;
    for(int i = 0; i < ctl->h; i++) {
      for(int j = 0; j < ctl->w; j++) {
        fb[bias + i * 400 + j] = ((uint32_t*)(ctl->pixels))[i * ctl->w + j];
      }
    }
    outl(SYNC_ADDR, 1);
  }
}

void __am_gpu_status(AM_GPU_STATUS_T *status) {
  status->ready = true;
}
