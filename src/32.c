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

static char sym_type(Elf32_Shdr* shdr, Elf32_Sym* symbol)
{
	uint8_t type;
	uint8_t bind;
	uint16_t shndx;
	Elf32_Shdr* sh;
	uint32_t sh_type;
	uint32_t sh_flags;

	type = ELF32_ST_TYPE(symbol->st_info);
	bind = ELF32_ST_BIND(symbol->st_info);
	shndx = sw16(symbol->st_shndx);
	if (type == STT_GNU_IFUNC)
		return 'i';
	else if (bind == STB_GNU_UNIQUE)
		return 'u';
	else if (bind == STB_WEAK)
	{
		if (type == STT_OBJECT)
			return shndx == SHN_UNDEF ? 'v' : 'V';
		else
			return shndx == SHN_UNDEF ? 'w' : 'W';
	}
	else if (shndx == SHN_ABS)
		return bind == STB_LOCAL ? 'a' : 'A';
	else if (shndx == SHN_COMMON)
		return 'C';
	else if (shndx == SHN_UNDEF)
		return 'U';
	else
	{
		sh = shdr + shndx;
		sh_type = sw32(sh->sh_type);
		sh_flags = sw32(sh->sh_flags);
		if (sh_flags & SHF_EXECINSTR && sh_flags & SHF_ALLOC)
			return bind == STB_LOCAL ? 't' : 'T';
		else if (sh_flags & SHF_WRITE && sh_flags & SHF_ALLOC)
		{
			if (sh_type == SHT_NOBITS)
				return bind == STB_LOCAL ? 'b' : 'B';
			else
				return bind == STB_LOCAL ? 'd' : 'D';
		}
		else if (!sh_flags)
			return 'N';
		else if (!(sh_flags & SHF_WRITE))
			return bind == STB_LOCAL ? 'r' : 'R';
		else
			return '?';
	}
}

static int valid_symtab(Elf32_Ehdr* ehdr, Elf32_Shdr* shdr, off_t fsize)
{
	Elf32_Off offset;
	uint32_t size;

	offset = sw32(shdr->sh_offset);
	size = sw32(shdr->sh_size);
	if (!offset || !size || offset + size > (uint32_t)fsize || !sw32(shdr->sh_link)
			|| sw32(shdr->sh_link) >= sw16(ehdr->e_shnum) || !sw32(shdr->sh_entsize))
		return 0;
	return 1;
}

static int valid_strtab(Elf32_Shdr* shdr, off_t fsize)
{
	Elf32_Off offset;
	uint32_t size;

	offset = sw32(shdr->sh_offset);
	size = sw32(shdr->sh_size);
	if (!offset || !size || sw32(shdr->sh_type) != SHT_STRTAB || offset + size > (uint32_t)fsize)
		return 0;
	return 1;
}

static int valid_symbol(Elf32_Ehdr* ehdr, Elf32_Sym* symbol, Elf32_Shdr* strtab)
{
	if (sw32(symbol->st_name) >= sw32(strtab->sh_size))
		return 0;
	if (sw16(symbol->st_shndx) < SHN_LORESERVE && sw16(symbol->st_shndx) >= sw16(ehdr->e_shnum))
		return 0;
	return 1;
}

static int valid_string(uint32_t start, char* strtab_off, uint32_t size)
{
	for (uint32_t i = start; i < size; i++)
	{
		if (strtab_off[i] == '\0')
			return 1;
	}
	return 0;
}

int save_symbols32(Elf32_Ehdr* ehdr, Nm* nm, off_t fsize)
{
	Elf32_Shdr* shdr;
	int symtab_index;
	Elf32_Shdr* symtab;
	Elf32_Shdr* strtab;
	char* strtab_off;
	Elf32_Sym* symbols;
	Elf32_Sym* symbol;
	size_t nb_symbols;

	shdr = (Elf32_Shdr*)((uint8_t*)ehdr + sw32(ehdr->e_shoff));
	symtab_index = find_symtab_index(shdr, sw16(ehdr->e_shnum));
	if (symtab_index == -1)
		return -1;
	if (!valid_symtab(ehdr, shdr + symtab_index, fsize))
		return -2;
	symtab = shdr + symtab_index;
	if (!valid_strtab(shdr + sw32(symtab->sh_link), fsize))
		return -2;
	strtab = shdr + sw32(symtab->sh_link);
	strtab_off = (char*)ehdr + sw32(strtab->sh_offset);
	symbols = (Elf32_Sym*)((uint8_t*)ehdr + sw32(symtab->sh_offset));
	nb_symbols = sw32(symtab->sh_size) / sw32(symtab->sh_entsize) - 1;
	nm->symbols = malloc(sizeof(Symbol) * nb_symbols);
	if (!nm->symbols)
		return 0;
	nm->nbsym = 0;
	for (size_t i = 0; i < nb_symbols; i++)
	{
		symbol = symbols + i + 1;
		if (!valid_symbol(ehdr, symbol, strtab))
			return -2;
		if ((nm->flags && DEBUG) || (ELF32_ST_TYPE(symbol->st_info) != STT_FILE &&
				ELF32_ST_TYPE(symbol->st_info) != STT_SECTION))
		{
			if (sw16(symbol->st_shndx) != SHN_UNDEF)
			{
				nm->symbols[nm->nbsym].value = ft_itoa(sw32(symbol->st_value), "0123456789abcdef");
				if (!nm->symbols[nm->nbsym].value)
					return 0;
			}
			else
				nm->symbols[nm->nbsym].value = NULL;
			nm->symbols[nm->nbsym].type = sym_type(shdr, symbol);
			if (sw32(symbol->st_name))
			{
				if (valid_string(sw32(symbol->st_name), strtab_off, sw32(strtab->sh_size)))
					nm->symbols[nm->nbsym].name = ft_strdup(strtab_off + sw32(symbol->st_name), 0);
				else
					nm->symbols[nm->nbsym].name = ft_strdup("(null)", 0);
			}
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
