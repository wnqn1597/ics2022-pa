#include <common.h>

#if defined(MULTIPROGRAM) && !defined(TIME_SHARING)
# define MULTIPROGRAM_YIELD() yield()
#else
# define MULTIPROGRAM_YIELD()
#endif

#define NAME(key) \
  [AM_KEY_##key] = #key,

//void exchangeFGPCB(int index);

static const char *keyname[256] __attribute__((used)) = {
  [AM_KEY_NONE] = "NONE",
  AM_KEYS(NAME)
};

static uint32_t fb_w;
static uint32_t fb_h;
void* get_finfo(int index, int property);

size_t serial_write(const void *buf, size_t offset, size_t len) {
  int i;
  for(i = 0; i < len; i++) putch(*((char*)buf + i));
  return len;
}

size_t events_read(void *buf, size_t offset, size_t len) {
  uint32_t val = get_keyboard_val();
  if(val == 0) return 0;
  if(val < 256) {
		// fg_pcb
		// exchangeFGPCB(val);
    const char *name = keyname[val];
    strcpy(buf, "ku ");
    strncpy(buf+3, name, len);
  }else {
    uint32_t masked = val & 0x7fff;
    const char *name = keyname[masked];
    strcpy(buf, "kd ");
    strncpy(buf+3, name, len);
  }
  return val;
}

size_t fbevt_read(void *buf, size_t offset, size_t len) {
  const char *msg = "mmap ok";
  strcpy(buf, msg);
  return 7;
}

size_t fbctl_read(void *buf, size_t offset, size_t len) {
  sprintf(buf, "%d %d", fb_w, fb_h);
  return len;
}

size_t fbctl_write(const void *buf, size_t offset, size_t len) {
  int i;
  for(i = 0; *((char*)buf+i) != '\0'; i++);
  char width[10], height[10];
  strncpy(width, buf, i); strncpy(height, buf+i+1, 10);
  fb_w = atoi(width); fb_h = atoi(height);
  return len;
}

size_t dispinfo_read(void *buf, size_t offset, size_t len) {
  uint32_t height = get_height();
  uint32_t width = get_width();
  return sprintf(buf, "WIDTH:%d\nHEIGHT:%d\n", width, height);
}

//static int count = 0;

size_t fb_write(const void *buf, size_t offset, size_t len) {
	uint32_t *fb = get_fb();
  uint32_t size = *((uint32_t*)get_finfo(5, 1));
  uint32_t *open_offset = (uint32_t*)get_finfo(5, 5);
  
  if(*open_offset + len > size) len -= *open_offset + len - size;
  memcpy(fb+*open_offset, buf, len*4);
  *open_offset += len;

  do_sync();

  return len;
}

void init_device() {
  Log("Initializing devices...");
  ioe_init();
}
