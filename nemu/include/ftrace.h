#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>
#include <elf.h>

# define Elf_Ehdr Elf32_Ehdr
# define Elf_Shdr Elf32_Shdr
# define Elf_Sym Elf32_Sym

void register_functab(const char *filename);
char* getFuncName(uint32_t addr);
void printFuncCall(uint32_t addr, int call);

