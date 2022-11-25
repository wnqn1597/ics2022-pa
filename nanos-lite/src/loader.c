#include <proc.h>
#include <elf.h>

#ifdef __LP64__
# define Elf_Ehdr Elf64_Ehdr
# define Elf_Phdr Elf64_Phdr
#else
# define Elf_Ehdr Elf32_Ehdr
# define Elf_Phdr Elf32_Phdr
#endif

size_t ramdisk_read(void *buf, size_t offset, size_t len);
size_t ramdisk_write(const void *buf, size_t offset, size_t len);

int fs_open(const char *pathname, int flags, int mode);
int fs_close(int fd);
void* get_finfo(int index, int property);

uintptr_t loader(PCB *pcb, const char *filename) {
  int fd = fs_open(filename, 0, 0);
	printf("loader\n");
  if(fd == -1 && filename != NULL) return -1;
  size_t offset = *((size_t*)get_finfo(fd, 2));
	printf("offset %08x\n", offset);
  Elf_Ehdr ehdr;
  size_t bias = ramdisk_read(&ehdr, offset, sizeof(Elf_Ehdr));
	printf("e\n");
  assert(*(uint32_t*)ehdr.e_ident == 0x464c457f);
  Elf_Phdr phdr[ehdr.e_phnum];
  ramdisk_read(phdr, offset + bias, ehdr.e_phnum * sizeof(Elf_Phdr));

 	printf("begin load\n");
  for(int i = 0; i < ehdr.e_phnum; i++) {
    if(phdr[i].p_type == PT_LOAD) {
			int lowerBound = phdr[i].p_vaddr & ~0xfff;
			int upperBound = (phdr[i].p_vaddr + phdr[i].p_memsz) & ~0xfff;
			int n_page = (upperBound - lowerBound) / PGSIZE + 1;
			char *vptr = (char*)lowerBound;
			for(int j = 0; j < n_page; j++) {
				map(&pcb->as, (void*)vptr, new_page(1), 0);
				vptr += PGSIZE;
			}

      ramdisk_read((void*)phdr[i].p_vaddr, offset + phdr[i].p_offset, phdr[i].p_memsz);
      memset((void*)(phdr[i].p_vaddr + phdr[i].p_filesz), 0, phdr[i].p_memsz - phdr[i].p_filesz);

			if(phdr[i].p_vaddr + phdr[i].p_memsz > pcb->max_brk) {
				pcb->max_brk = phdr[i].p_vaddr + phdr[i].p_memsz;
			}
    }
  }
  fs_close(fd);
  return ehdr.e_entry;
}

void naive_uload(PCB *pcb, const char *filename) {
  uintptr_t entry = loader(pcb, filename);
	if(entry == -1) return;
  Log("Jump to entry = %p", entry);
  ((void(*)())entry) ();
}

