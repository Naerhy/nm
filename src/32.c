#include "nm.h"

static int find_symtab_index(Elf32_Shdr* shdr, uint16_t size)
{
	for (uint16_t i = 0; i < size; i++)
	{
		if (sw32((shdr + i)->sh_type) == SHT_SYMTAB)
			return (int)i;
	}
	return -1;
}

static char sym_type(Elf32_Shdr* shdr, char* section_names, Elf32_Sym* symbol)
{
	uint16_t shndx;

	shndx = sw16(symbol->st_shndx);
	if (ELF32_ST_TYPE(symbol->st_info) == STT_GNU_IFUNC)
		return 'i';
	else if (ELF32_ST_BIND(symbol->st_info) == STB_GNU_UNIQUE)
		return 'u';
	else if (ELF32_ST_BIND(symbol->st_info) == STB_WEAK)
	{
		if (ELF32_ST_TYPE(symbol->st_info) == STT_OBJECT)
			return shndx != SHN_UNDEF ? 'V' : 'v';
		else
			return shndx != SHN_UNDEF ? 'W' : 'w';
	}
	else if (shndx == SHN_ABS)
		return ELF32_ST_BIND(symbol->st_info) == STB_GLOBAL ? 'A' : 'a';
	else if (shndx == SHN_COMMON)
		return 'C';
	else if (shndx == SHN_UNDEF)
		return 'U';
	else if (!ft_strcmp(section_names + sw32((shdr + shndx)->sh_name), ".debug", 0)
			|| !ft_strcmp(section_names + sw32((shdr + shndx)->sh_name), ".line", 0))
		return 'N';
	else if (sw32((shdr + shndx)->sh_flags) & SHF_EXECINSTR)
		return ELF32_ST_BIND(symbol->st_info) != STB_LOCAL ? 'T' : 't';
	else if (sw32((shdr + shndx)->sh_flags) & SHF_WRITE
			&& sw32((shdr + shndx)->sh_flags) & SHF_ALLOC)
	{
		if (sw32((shdr + shndx)->sh_type) == SHT_NOBITS)
			return ELF32_ST_BIND(symbol->st_info) != STB_LOCAL ? 'B' : 'b';
		else
			return ELF32_ST_BIND(symbol->st_info) != STB_LOCAL ? 'D' : 'd';
	}
	else if (!(sw32((shdr + shndx)->sh_flags) & SHF_WRITE))
		return ELF32_ST_BIND(symbol->st_info) != STB_LOCAL ? 'R' : 'r';
	else
		return '?';
}

static int valid_shstrtab(Elf32_Ehdr* ehdr, Elf32_Shdr* shdr, off_t fsize)
{
	Elf32_Off offset;
	uint32_t size;

	offset = sw32(shdr->sh_offset);
	size = sw32(shdr->sh_size);
	if (!offset || !size || offset + size > (Elf32_Off)fsize || *((uint8_t*)ehdr + offset)
			|| *((uint8_t*)ehdr + offset + size - 1))
		return 0;
	return 1;
}

static int valid_symtab(Elf32_Ehdr* ehdr, Elf32_Shdr* shdr, off_t fsize)
{
	Elf32_Off offset;
	uint32_t size;

	offset = sw32(shdr->sh_offset);
	size = sw32(shdr->sh_size);
	if (!offset || !size || offset + size > (Elf32_Off)fsize || !sw32(shdr->sh_link)
			|| sw32(shdr->sh_link) > sw16(ehdr->e_shnum) || !sw32(shdr->sh_entsize))
		return 0;
	return 1;
}

static int valid_strtab(Elf32_Ehdr* ehdr, Elf32_Shdr* shdr, off_t fsize)
{
	Elf32_Off offset;
	uint32_t size;

	offset = sw32(shdr->sh_offset);
	size = sw32(shdr->sh_size);
	if (!offset || !size || offset + size > (Elf32_Off)fsize || *((uint8_t*)ehdr + offset)
			|| *((uint8_t*)ehdr + offset + size - 1))
		return 0;
	return 1;
}

int save_symbols32(Elf32_Ehdr* ehdr, Nm* nm, off_t fsize)
{
	Elf32_Shdr* shdr;
	char* section_names;
	int symtab_index;
	Elf32_Shdr* symtab;
	char* symbol_names;
	Elf32_Sym* symbols;
	Elf32_Sym* symbol;
	size_t nb_symbols;

	shdr = (Elf32_Shdr*)((uint8_t*)ehdr+ sw32(ehdr->e_shoff));
	if (!valid_shstrtab(ehdr, shdr + sw16(ehdr->e_shstrndx), fsize))
		return -2;
	section_names = (char*)ehdr+ sw32((shdr + sw16(ehdr->e_shstrndx))->sh_offset);
	symtab_index = find_symtab_index(shdr, sw16(ehdr->e_shnum));
	if (symtab_index == -1)
		return -1;
	if (!valid_symtab(ehdr, shdr + symtab_index, fsize))
		return -2;
	symtab = shdr + symtab_index;
	if (!valid_strtab(ehdr, shdr + sw32(symtab->sh_link), fsize))
		return -2;
	symbol_names = (char*)ehdr + sw32((shdr + sw32(symtab->sh_link))->sh_offset);
	symbols = (Elf32_Sym*)((uint8_t*)ehdr + sw32(symtab->sh_offset));
	nb_symbols = sw32(symtab->sh_size) / sw32(symtab->sh_entsize) - 1;
	nm->symbols = malloc(sizeof(Symbol) * nb_symbols);
	if (!nm->symbols)
		return 0;
	nm->nbsym = 0;
	for (size_t i = 0; i < nb_symbols; i++)
	{
		symbol = symbols + i + 1;
		if (ELF32_ST_TYPE(symbol->st_info) != STT_FILE &&
				ELF32_ST_TYPE(symbol->st_info) != STT_SECTION)
		{
			if (sw16(symbol->st_shndx) != SHN_UNDEF)
			{
				nm->symbols[nm->nbsym].value = ft_itoa(sw32(symbol->st_value), "0123456789abcdef");
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
