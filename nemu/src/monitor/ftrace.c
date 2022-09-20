#ifdef CONFIG_FTRACE
#include <ftrace.h>

Elf_Shdr shdr[32];
Elf_Sym syms[128];
char shstrtab[256];
char strtab[1024];

typedef struct{
  uint32_t value;
  uint32_t size;
  char name[32];
}FuncTab;

static FuncTab functab[128];
static int nr_functab = 0;
static int call_level = 0;
int __attribute__((unused)) u;

void register_functab(const char *filename){
	printf("filename=%s--\n", filename);
	FILE *f = fopen(filename, "rb");
	assert(f);
	
	Elf_Ehdr ehdr;
	fseek(f, 0, SEEK_SET);
	u = fread(&ehdr, sizeof(Elf_Ehdr), 1, f);

	fseek(f, ehdr.e_shoff, SEEK_SET);
	u = fread(shdr, sizeof(Elf_Shdr), ehdr.e_shnum, f);

	fseek(f, shdr[ehdr.e_shstrndx].sh_offset, SEEK_SET);
	u = fread(shstrtab, 1, shdr[ehdr.e_shstrndx].sh_size, f);
	
	int strndx = -1, symndx = -1;
	for(int i = 0; i < ehdr.e_shnum; i++){
		char *name = &shstrtab[shdr[i].sh_name];
		if(strcmp(name, ".symtab") == 0) symndx = i;
		if(strcmp(name, ".strtab") == 0) strndx = i;
	}
	int len = shdr[symndx].sh_size / shdr[symndx].sh_entsize;
	fseek(f, shdr[symndx].sh_offset, SEEK_SET);
	u = fread(&syms, sizeof(Elf_Sym), len, f);
	fseek(f, shdr[strndx].sh_offset, SEEK_SET);
	u = fread(strtab, 1, shdr[strndx].sh_size, f);
	
	for(int i = 0; i < len; i++){
		if(syms[i].st_info == 0x12) {
			functab[nr_functab].value = syms[i].st_value;
			functab[nr_functab].size  = syms[i].st_size;
			strcpy(functab[nr_functab].name, &strtab[syms[i].st_name]);
			nr_functab++;
		}
	}
}

char* getFuncName(uint32_t addr){
	for(int i = 0; i < nr_functab; i++){
		if(addr >= functab[i].value && addr < functab[i].value + functab[i].size){
			return functab[i].name;
		}
	}
	return "NULL";
}

void printFuncCall(uint32_t addr, int call){
	char *funcname = getFuncName(addr);
	if(call == 0) call_level--;
	for(int i = 0; i < call_level; i++) printf("  ");
	if(call == 1){
		printf("call");
		call_level++;
	}else printf("ret");
	
	printf("[%s @ %08x]\n", funcname, addr);
}
#endif
