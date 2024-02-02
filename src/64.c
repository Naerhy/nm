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

static int validate_sh(Elf64_Shdr* sh, off_t fsize)
{
	if (sh->sh_offset > (Elf64_Off)fsize || sh->sh_offset + sh->sh_size > (Elf64_Off)fsize)
		return 0;
	return 1;
}

int save_symbols64(Elf64_Ehdr* header, Nm* nm, off_t fsize)
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
	if (!validate_sh(sht + sw16(header->e_shstrndx), fsize))
		return -2;
	shstrtab = (unsigned char*)header + sw64((sht + sw16(header->e_shstrndx))->sh_offset);
	index = symtab_index(sht, sw16(header->e_shnum));
	if (index == -1)
		return -1;
	sh_symtab = sht + index;
	if (!validate_sh(sh_symtab, fsize) || !validate_sh(sht + sw32(sh_symtab->sh_link), fsize))
		return -2;
	strtab = (unsigned char*)header + sw64((sht + sw32(sh_symtab->sh_link))->sh_offset);
	symtab = (Elf64_Sym*)((unsigned char*)header + sw64(sh_symtab->sh_offset));
	for (uint64_t j = sw64(sh_symtab->sh_entsize); j < sw64(sh_symtab->sh_size); j += sw64(sh_symtab->sh_entsize))
	{
		symbol = symtab + j / sw64(sh_symtab->sh_entsize);
		if (ELF64_ST_TYPE(symbol->st_info) != STT_FILE && ELF64_ST_TYPE(symbol->st_info) != STT_SECTION)
		{
			symnode = malloc(sizeof(Symbol));
			if (!symnode)
				return 0;
			if (sw16(symbol->st_shndx) != SHN_UNDEF)
			{
				symnode->value = ft_itoa(sw64(symbol->st_value), "0123456789abcdef");
				if (!symnode->value)
				{
					free(symnode);
					return 0;
				}
			}
			else
				symnode->value = NULL;
			symnode->type = sym_type(sht, shstrtab, symbol);
			symnode->name = ft_strdup(sw32(symbol->st_name) ? (char*)strtab + sw32(symbol->st_name) : "", 0);
			if (!symnode->name)
			{
				free(symnode->value);
				free(symnode);
				return 0;
			}
			lknode = lklist_create((void*)symnode);
			// TODO: call free_symbol from nm.c instead
			if (!lknode)
			{
				free(symnode->value);
				free(symnode->name);
				free(symnode);
				return 0;
			}
			lklist_add(&nm->symbols, lknode);
		}
	}
	return 1;
}
