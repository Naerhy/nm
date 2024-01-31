#include "nm.h"

static int symtab_index(Elf32_Shdr* sht, uint16_t size)
{
	for (uint16_t i = 0; i < size; i++)
	{
		if (sw32((sht + i)->sh_type) == SHT_SYMTAB)
			return (int)i;
	}
	return -1;
}

static char sym_type(Elf32_Shdr* sht, unsigned char* shstrtab, Elf32_Sym* symbol)
{
	uint16_t st_shndx;

	st_shndx = sw16(symbol->st_shndx);
	if (ELF32_ST_TYPE(symbol->st_info) == STT_GNU_IFUNC)
		return 'i';
	else if (ELF32_ST_BIND(symbol->st_info) == STB_GNU_UNIQUE)
		return 'u';
	else if (ELF32_ST_BIND(symbol->st_info) == STB_WEAK)
	{
		if (ELF32_ST_TYPE(symbol->st_info) == STT_OBJECT)
			return st_shndx != SHN_UNDEF ? 'V' : 'v';
		else
			return st_shndx != SHN_UNDEF ? 'W' : 'w';
	}
	else if (st_shndx == SHN_ABS)
		return 'A';
	else if (st_shndx == SHN_COMMON)
		return 'C';
	else if (st_shndx == SHN_UNDEF)
		return 'U';
	else if (!strcmp((char*)shstrtab + sw32((sht + st_shndx)->sh_name), ".debug")
			|| !strcmp((char*)shstrtab + sw32((sht + st_shndx)->sh_name), ".line"))
		return 'N';
	else if (sw32((sht + st_shndx)->sh_flags) & SHF_EXECINSTR)
		return ELF32_ST_BIND(symbol->st_info) != STB_LOCAL ? 'T' : 't';
	else if (sw32((sht + st_shndx)->sh_flags) & SHF_WRITE
			&& sw32((sht + st_shndx)->sh_flags) & SHF_ALLOC)
	{
		if (sw32((sht + st_shndx)->sh_type) == SHT_NOBITS)
			return ELF32_ST_BIND(symbol->st_info) != STB_LOCAL ? 'B' : 'b';
		else
			return ELF32_ST_BIND(symbol->st_info) != STB_LOCAL ? 'D' : 'd';
	}
	else if (!(sw32((sht + st_shndx)->sh_flags) & SHF_WRITE))
		return ELF32_ST_BIND(symbol->st_info) != STB_LOCAL ? 'R' : 'r';
	else
		return '?';
}

void print_symbols32(Elf32_Ehdr* header)
{
	Elf32_Shdr* sht;
	unsigned char* shstrtab;
	int index;
	Elf32_Shdr* sh_symtab;
	Elf32_Sym* symtab;
	unsigned char* strtab;
	Elf32_Sym* symbol;

	sht = (Elf32_Shdr*)((unsigned char*)header + sw32(header->e_shoff));
	shstrtab = (unsigned char*)header + sw32((sht + sw16(header->e_shstrndx))->sh_offset);
	index = symtab_index(sht, sw16(header->e_shnum));
	if (index == -1)
		return ;
	sh_symtab = sht + index;
	strtab = (unsigned char*)header + sw32((sht + sw32(sh_symtab->sh_link))->sh_offset);
	symtab = (Elf32_Sym*)((unsigned char*)header + sw32(sh_symtab->sh_offset));
	for (uint32_t j = sw32(sh_symtab->sh_entsize); j < sw32(sh_symtab->sh_size); j += sw32(sh_symtab->sh_entsize))
	{
		symbol = symtab + j / sw32(sh_symtab->sh_entsize);
		if (ELF32_ST_TYPE(symbol->st_info) != STT_FILE && ELF32_ST_TYPE(symbol->st_info) != STT_SECTION)
		{

			// print value
			if (sw16(symbol->st_shndx) == SHN_UNDEF)
				printf("%8s", "");
			else
				printf("%08x", sw32(symbol->st_value));
			printf(" ");

			// print type
			printf("%c", sym_type(sht, shstrtab, symbol));
			printf(" ");

			// print name
			if (symbol->st_name)
				printf("%s", strtab + sw32(symbol->st_name));
			else
				printf("(null)");
			printf("\n");

		}
	}
}
