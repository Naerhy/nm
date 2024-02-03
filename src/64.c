#include "nm.h"

static int find_symtab_index(Elf64_Shdr* shdr, uint16_t size)
{
	for (uint16_t i = 0; i < size; i++)
	{
		if (sw32((shdr + i)->sh_type) == SHT_SYMTAB)
			return (int)i;
	}
	return -1;
}

static char sym_type(Elf64_Shdr* shdr, char* section_names, Elf64_Sym* symbol)
{
	uint16_t shndx;

	shndx = sw16(symbol->st_shndx);
	if (ELF64_ST_TYPE(symbol->st_info) == STT_GNU_IFUNC)
		return 'i';
	else if (ELF64_ST_BIND(symbol->st_info) == STB_GNU_UNIQUE)
		return 'u';
	else if (ELF64_ST_BIND(symbol->st_info) == STB_WEAK)
	{
		if (ELF64_ST_TYPE(symbol->st_info) == STT_OBJECT)
			return shndx != SHN_UNDEF ? 'V' : 'v';
		else
			return shndx != SHN_UNDEF ? 'W' : 'w';
	}
	else if (shndx == SHN_ABS)
		return 'A';
	else if (shndx == SHN_COMMON)
		return 'C';
	else if (shndx == SHN_UNDEF)
		return 'U';
	else if (!ft_strcmp(section_names + sw32((shdr + shndx)->sh_name), ".debug", 0)
			|| !ft_strcmp(section_names + sw32((shdr + shndx)->sh_name), ".line", 0))
		return 'N';
	else if (sw64((shdr + shndx)->sh_flags) & SHF_EXECINSTR)
		return ELF64_ST_BIND(symbol->st_info) != STB_LOCAL ? 'T' : 't';
	else if (sw64((shdr + shndx)->sh_flags) & SHF_WRITE
			&& sw64((shdr + shndx)->sh_flags) & SHF_ALLOC)
	{
		if (sw32((shdr + shndx)->sh_type) == SHT_NOBITS)
			return ELF64_ST_BIND(symbol->st_info) != STB_LOCAL ? 'B' : 'b';
		else
			return ELF64_ST_BIND(symbol->st_info) != STB_LOCAL ? 'D' : 'd';
	}
	else if (!(sw64((shdr + shndx)->sh_flags) & SHF_WRITE))
		return ELF64_ST_BIND(symbol->st_info) != STB_LOCAL ? 'R' : 'r';
	else
		return '?';
}

static int valid_shstrtab(Elf64_Ehdr* ehdr, Elf64_Shdr* shdr, off_t fsize)
{
	Elf64_Off offset;
	uint64_t size;

	offset = sw64(shdr->sh_offset);
	size = sw64(shdr->sh_size);
	if (!offset || !size || offset + size > (Elf64_Off)fsize || *((uint8_t*)ehdr + offset)
			|| *((uint8_t*)ehdr + offset + size - 1))
		return 0;
	return 1;
}

static int valid_symtab(Elf64_Ehdr* ehdr, Elf64_Shdr* shdr, off_t fsize)
{
	Elf64_Off offset;
	uint64_t size;

	offset = sw64(shdr->sh_offset);
	size = sw64(shdr->sh_size);
	if (!offset || !size || offset + size > (Elf64_Off)fsize || !sw32(shdr->sh_link)
			|| sw32(shdr->sh_link) > sw16(ehdr->e_shnum) || !sw64(shdr->sh_entsize))
		return 0;
	return 1;
}

static int valid_strtab(Elf64_Ehdr* ehdr, Elf64_Shdr* shdr, off_t fsize)
{
	Elf64_Off offset;
	uint64_t size;

	offset = sw64(shdr->sh_offset);
	size = sw64(shdr->sh_size);
	if (!offset || !size || offset + size > (Elf64_Off)fsize || *((uint8_t*)ehdr + offset)
			|| *((uint8_t*)ehdr + offset + size - 1))
		return 0;
	return 1;
}

int save_symbols64(Elf64_Ehdr* ehdr, Nm* nm, off_t fsize)
{
	Elf64_Shdr* shdr;
	char* section_names;
	int symtab_index;
	Elf64_Shdr* symtab;
	char* symbol_names;
	Elf64_Sym* symbols;
	Elf64_Sym* symbol;
	size_t nb_symbols;

	shdr = (Elf64_Shdr*)((uint8_t*)ehdr + sw64(ehdr->e_shoff));
	if (!valid_shstrtab(ehdr, shdr + sw16(ehdr->e_shstrndx), fsize))
		return -2;
	section_names = (char*)ehdr + sw64((shdr + sw16(ehdr->e_shstrndx))->sh_offset);
	symtab_index = find_symtab_index(shdr, sw16(ehdr->e_shnum));
	if (symtab_index == -1)
		return -1;
	if (!valid_symtab(ehdr, shdr + symtab_index, fsize))
		return -2;
	symtab = shdr + symtab_index;
	if (!valid_strtab(ehdr, shdr + sw32(symtab->sh_link), fsize))
		return -2;
	symbol_names = (char*)ehdr + sw64((shdr + sw32(symtab->sh_link))->sh_offset);
	symbols = (Elf64_Sym*)((uint8_t*)ehdr + sw64(symtab->sh_offset));
	nb_symbols = sw64(symtab->sh_size) / sw64(symtab->sh_entsize) - 1;
	nm->symbols = malloc(sizeof(Symbol) * nb_symbols);
	if (!nm->symbols)
		return 0;
	nm->nbsym = 0;
	for (size_t i = 0; i < nb_symbols; i++)
	{
		symbol = symbols + i + 1;
		if (ELF64_ST_TYPE(symbol->st_info) != STT_FILE &&
				ELF64_ST_TYPE(symbol->st_info) != STT_SECTION)
		{
			if (sw16(symbol->st_shndx) != SHN_UNDEF)
			{
				nm->symbols[nm->nbsym].value = ft_itoa(sw64(symbol->st_value), "0123456789abcdef");
				if (!nm->symbols[nm->nbsym].value)
					return 0;
			}
			else
				nm->symbols[nm->nbsym].value = NULL;
			nm->symbols[nm->nbsym].type = sym_type(shdr, section_names, symbol);
			if (sw32(symbol->st_name))
				nm->symbols[nm->nbsym].name = ft_strdup(symbol_names + sw32(symbol->st_name), 0);
			else
				nm->symbols[nm->nbsym].name = ft_strdup("", 0);
			if (!nm->symbols[nm->nbsym].name)
			{
				free(nm->symbols[nm->nbsym].value);
				return 0;
			}
			nm->nbsym++;
		}
	}
	return 1;
}
