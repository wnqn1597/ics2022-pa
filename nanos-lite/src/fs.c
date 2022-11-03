#include <fs.h>

typedef struct {
  char *name;
  size_t size;
  size_t disk_offset;
  ReadFn read;
  WriteFn write;
  size_t open_offset;
} Finfo;

enum {FD_STDIN, FD_STDOUT, FD_STDERR, FD_FBEVT, FD_FBCTL, FD_FB, FD_DISP, FD_EVENT};

size_t invalid_read(void *buf, size_t offset, size_t len) {
  panic("invalid read");
  return 0;
}

size_t invalid_write(const void *buf, size_t offset, size_t len) {
  panic("invalid write");
  return 0;
}

size_t ramdisk_read(void *buf, size_t offset, size_t len);
size_t ramdisk_write(const void *buf, size_t offset, size_t len);
size_t serial_write(const void *buf, size_t offset, size_t len);
size_t events_read(void *buf, size_t offset, size_t len);
size_t dispinfo_read(void *buf, size_t offset, size_t len);
size_t fbctl_write(const void *buf, size_t offset, size_t len);
size_t fbevt_read(void *buf, size_t offset, size_t len);
size_t fb_write(const void *buf, size_t offset, size_t len);

/* This is the information about all files in disk. */
static Finfo file_table[] __attribute__((used)) = {
  [FD_STDIN]  = {"stdin", 0, 0, invalid_read, invalid_write, 0},
  [FD_STDOUT] = {"stdout", 0, 0, invalid_read, serial_write, 0},
  [FD_STDERR] = {"stderr", 0, 0, invalid_read, serial_write, 0},
  [FD_FBEVT]  = {"/dev/fbevt", 0, 0, fbevt_read, invalid_write, 0},
  [FD_FBCTL]  = {"/dev/fbctl", 0, 0, invalid_read, fbctl_write, 0},
  [FD_FB]     = {"/dev/fb", 0, 0, invalid_read, fb_write, 0},
  [FD_DISP]   = {"/proc/dispinfo", 0, 0, dispinfo_read, invalid_write, 0},
  [FD_EVENT]  = {"/dev/events", 0, 0, events_read, invalid_write, 0},
#include "files.h"
};

void* get_finfo(int index, int property) {
  unsigned length = sizeof(file_table) / sizeof(Finfo);
  if(index >= 0 && index < length) {
    switch(property){
      case 1: return (void*)&file_table[index].size;
      case 2: return (void*)&file_table[index].disk_offset;
      case 3: return file_table[index].read;
      case 4: return file_table[index].write;
      case 5: return (void*)&file_table[index].open_offset;
      default: assert(0);
    }
  }else assert(0);
}

int fs_open(const char *pathname, int flags, int mode) {
  int length = sizeof(file_table) / sizeof(Finfo);
  int i;
  for(i = 0; i < length; i++) {
    if(strcmp(pathname, file_table[i].name) == 0) break;
  }
  return i == length ? -1 : i;
}

int fs_write(int fd, const void *buf, size_t len) {
  size_t offset = file_table[fd].disk_offset + file_table[fd].open_offset;
  file_table[fd].open_offset += len;
  if(file_table[fd].open_offset > file_table[fd].size){
    //printf("Write out of the file.\n");
    len -= (file_table[fd].open_offset - file_table[fd].size);
    file_table[fd].open_offset = file_table[fd].size;
  }
  return ramdisk_write(buf, offset, len);
}

int fs_read(int fd, void *buf, size_t len) {
  size_t offset = file_table[fd].disk_offset + file_table[fd].open_offset;
  file_table[fd].open_offset += len;
  if(file_table[fd].open_offset > file_table[fd].size){
    //printf("Read out of the file.\n");
    len -= (file_table[fd].open_offset - file_table[fd].size);
    file_table[fd].open_offset = file_table[fd].size;
  }
  return ramdisk_read(buf, offset, len);
}

int fs_close(int fd) {
  file_table[fd].open_offset = file_table[fd].disk_offset;
  return 0;
}

int fs_lseek(int fd, size_t offset, int whence) {
  size_t old_offset = file_table[fd].open_offset;
  switch(whence) {
    case SEEK_SET: file_table[fd].open_offset = offset;break; 
    case SEEK_CUR: file_table[fd].open_offset += offset;break;
    case SEEK_END: file_table[fd].open_offset = file_table[fd].size + offset;break;
    default: panic("Unhandled whence = %d", whence);
  }
  if(file_table[fd].open_offset >= 0 && file_table[fd].open_offset <= file_table[fd].size) {
    return file_table[fd].open_offset;
  }else{
    file_table[fd].open_offset = old_offset;
    panic("Offset out of bound.");
  }
}

void init_fs() {
  // TODO: initialize the size of /dev/fb
  char buf[10];
  (file_table[6].read)(buf, 0, 0);
  
  int w = atoi(buf);
  int i;
  for(i = 0; buf[i] != '\n'; i++);
  i++;
  int h = atoi(buf + i);
  file_table[FD_FB].size = w * h * 4;
}

