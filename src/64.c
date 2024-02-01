#include "nm.h"

static int symtab_index(Elf64_Shdr* sht, uint16_t size)
{
	for (uint16_t i = 0; i < size; i++)
	{
		if (sw32((sht + i)->sh_type) == SHT_SYMTAB)
			return (int)i;
	}
	return -1;
}

static char sym_type(Elf64_Shdr* sht, unsigned char* shstrtab, Elf64_Sym* symbol)
{
	uint16_t st_shndx;

	st_shndx = sw16(symbol->st_shndx);
	if (ELF64_ST_TYPE(symbol->st_info) == STT_GNU_IFUNC)
		return 'i';
	else if (ELF64_ST_BIND(symbol->st_info) == STB_GNU_UNIQUE)
		return 'u';
	else if (ELF64_ST_BIND(symbol->st_info) == STB_WEAK)
	{
		if (ELF64_ST_TYPE(symbol->st_info) == STT_OBJECT)
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
	else if (!ft_strcmp((char*)shstrtab + sw32((sht + st_shndx)->sh_name), ".debug", 0)
			|| !ft_strcmp((char*)shstrtab + sw32((sht + st_shndx)->sh_name), ".line", 0))
		return 'N';
	else if (sw64((sht + st_shndx)->sh_flags) & SHF_EXECINSTR)
		return ELF64_ST_BIND(symbol->st_info) != STB_LOCAL ? 'T' : 't';
	else if (sw64((sht + st_shndx)->sh_flags) & SHF_WRITE
			&& sw64((sht + st_shndx)->sh_flags) & SHF_ALLOC)
	{
		if (sw32((sht + st_shndx)->sh_type) == SHT_NOBITS)
			return ELF64_ST_BIND(symbol->st_info) != STB_LOCAL ? 'B' : 'b';
		else
			return ELF64_ST_BIND(symbol->st_info) != STB_LOCAL ? 'D' : 'd';
	}
	else if (!(sw64((sht + st_shndx)->sh_flags) & SHF_WRITE))
		return ELF64_ST_BIND(symbol->st_info) != STB_LOCAL ? 'R' : 'r';
	else
		return '?';
}

void save_symbols64(Elf64_Ehdr* header, Nm* nm)
{
	Elf64_Shdr* sht;
	unsigned char* shstrtab;
	int index;
	Elf64_Shdr* sh_symtab;
	Elf64_Sym* symtab;
	unsigned char* strtab;
	Elf64_Sym* symbol;
	Symbol* symnode;
	LkList* lknode;

	sht = (Elf64_Shdr*)((unsigned char*)header + sw64(header->e_shoff));
	shstrtab = (unsigned char*)header + sw64((sht + sw16(header->e_shstrndx))->sh_offset);
	index = symtab_index(sht, sw16(header->e_shnum));
	if (index == -1)
		return ;
	sh_symtab = sht + index;
	strtab = (unsigned char*)header + sw64((sht + sw32(sh_symtab->sh_link))->sh_offset);
	symtab = (Elf64_Sym*)((unsigned char*)header + sw64(sh_symtab->sh_offset));
	for (uint64_t j = sw64(sh_symtab->sh_entsize); j < sw64(sh_symtab->sh_size); j += sw64(sh_symtab->sh_entsize))
	{
		symbol = symtab + j / sw64(sh_symtab->sh_entsize);
		if (ELF64_ST_TYPE(symbol->st_info) != STT_FILE && ELF64_ST_TYPE(symbol->st_info) != STT_SECTION)
		{
			symnode = malloc(sizeof(Symbol));
			// TODO: handle error
			if (!symnode)
				return ;
			symnode->value = sw16(symbol->st_shndx) == SHN_UNDEF ? NULL : ft_itoa(sw64(symbol->st_value), "0123456789abcdef");
			symnode->type = sym_type(sht, shstrtab, symbol);
			// check return error
			symnode->name = sw32(symbol->st_name) ? ft_strdup((char*)strtab + sw32(symbol->st_name), 0) : NULL;
			lknode = lklist_create((void*)symnode);
			// check error return
			// check double pointer -> better to pass it in parent function parameter?
			lklist_add(&nm->symbols, lknode);
		}
	}
}
