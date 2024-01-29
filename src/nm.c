#include "nm.h"

static unsigned char check_intpr(unsigned char const* intpr, off_t fsize)
{
	if (fsize < EI_NIDENT || intpr[EI_MAG0] != ELFMAG0 || intpr[EI_MAG1] != ELFMAG1
			|| intpr[EI_MAG2] != ELFMAG2 || intpr[EI_MAG3] != ELFMAG3)
		return ELFCLASSNONE;
	return intpr[EI_CLASS];
}

static void handle_32bits(Elf32_Ehdr* header)
{
	Elf32_Shdr* sht;
	unsigned char* shstrtab;
	Elf32_Shdr* sh_symtab;
	Elf32_Sym* symtab;
	unsigned char* strtab;
	Elf32_Sym* symbol;

	if (header->e_ehsize != 52 || !header->e_shoff)
		return ;
	sht = (Elf32_Shdr*)((unsigned char*)header + header->e_shoff);
	shstrtab = (unsigned char*)header + (sht + header->e_shstrndx)->sh_offset;
	for (uint16_t i = 0; i < header->e_shnum; i++)
	{
		if (!strcmp(".symtab", (char*)shstrtab + (sht + i)->sh_name))
		{
			sh_symtab = sht + i;
			strtab = (unsigned char*)header + (sht + sh_symtab->sh_link)->sh_offset;
			symtab = (Elf32_Sym*)((unsigned char*)header + sh_symtab->sh_offset);
			for (uint32_t j = sh_symtab->sh_entsize; j < sh_symtab->sh_size; j += sh_symtab->sh_entsize)
			{
				symbol = symtab + j / sh_symtab->sh_entsize;
				if (ELF32_ST_TYPE(symbol->st_info) != STT_FILE && ELF32_ST_TYPE(symbol->st_info) != STT_SECTION)
				{

					// print value
					if (symbol->st_shndx == SHN_UNDEF)
						printf("%8s", "");
					else
						printf("%08x", symbol->st_value);
					printf(" ");

					// print type
					if (ELF32_ST_TYPE(symbol->st_info) == STT_GNU_IFUNC)
						printf("i");
					else if (ELF32_ST_BIND(symbol->st_info) == STB_GNU_UNIQUE)
						printf("u");
					else if (ELF32_ST_BIND(symbol->st_info) == STB_WEAK)
					{
						if (ELF32_ST_TYPE(symbol->st_info) == STT_OBJECT)
							printf(symbol->st_shndx != SHN_UNDEF ? "V" : "v");
						else
						{
							printf(symbol->st_shndx != SHN_UNDEF ? "W" : "w");
						}
					}
					else if (symbol->st_shndx == SHN_ABS)
						printf("A");
					else if (symbol->st_shndx == SHN_COMMON)
						printf("C");
					else if (symbol->st_shndx == SHN_UNDEF)
						printf("U");
					else if (!strcmp((char*)shstrtab + (sht + symbol->st_shndx)->sh_name, ".debug")
							|| !strcmp((char*)shstrtab + (sht + symbol->st_shndx)->sh_name, ".line"))
						printf("N");
					else if ((sht + symbol->st_shndx)->sh_flags & SHF_EXECINSTR)
						printf(ELF32_ST_BIND(symbol->st_info) != STB_LOCAL ? "T" : "t");
					else if ((sht + symbol->st_shndx)->sh_flags & SHF_WRITE
							&& (sht + symbol->st_shndx)->sh_flags & SHF_ALLOC)
					{
						if ((sht + symbol->st_shndx)->sh_type == SHT_NOBITS)
							printf(ELF32_ST_BIND(symbol->st_info) != STB_LOCAL ? "B" : "b");
						else
							printf(ELF32_ST_BIND(symbol->st_info) != STB_LOCAL ? "D" : "d");
					}
					else if (!((sht + symbol->st_shndx)->sh_flags & SHF_WRITE))
						printf(ELF32_ST_BIND(symbol->st_info) != STB_LOCAL ? "R" : "r");
					else
						printf("?");
					printf(" ");

					// print name
					if (symbol->st_name)
						printf("%s", strtab + symbol->st_name);
					else
						printf("(null)");
					printf("\n");

				}
			}
		}
	}
}

static void handle_64bits(Elf64_Ehdr* header)
{
	Elf64_Shdr* sht;
	unsigned char* shstrtab;
	Elf64_Shdr* sh_symtab;
	Elf64_Sym* symtab;
	unsigned char* strtab;
	Elf64_Sym* symbol;

	if (header->e_ehsize != 64 || !header->e_shoff)
		return ;
	sht = (Elf64_Shdr*)((unsigned char*)header + header->e_shoff);
	shstrtab = (unsigned char*)header + (sht + header->e_shstrndx)->sh_offset;
	for (uint16_t i = 0; i < header->e_shnum; i++)
	{
		if (!strcmp(".symtab", (char*)shstrtab + (sht + i)->sh_name))
		{
			sh_symtab = sht + i;
			strtab = (unsigned char*)header + (sht + sh_symtab->sh_link)->sh_offset;
			symtab = (Elf64_Sym*)((unsigned char*)header + sh_symtab->sh_offset);
			for (uint64_t j = sh_symtab->sh_entsize; j < sh_symtab->sh_size; j += sh_symtab->sh_entsize)
			{
				symbol = symtab + j / sh_symtab->sh_entsize;
				if (ELF64_ST_TYPE(symbol->st_info) != STT_FILE && ELF64_ST_TYPE(symbol->st_info) != STT_SECTION)
				{

					// print value
					if (symbol->st_shndx == SHN_UNDEF)
						printf("%16s", "");
					else
						printf("%016lx", symbol->st_value);
					printf(" ");

					// print type
					if (ELF64_ST_TYPE(symbol->st_info) == STT_GNU_IFUNC)
						printf("i");
					else if (ELF64_ST_BIND(symbol->st_info) == STB_GNU_UNIQUE)
						printf("u");
					else if (ELF64_ST_BIND(symbol->st_info) == STB_WEAK)
					{
						if (ELF64_ST_TYPE(symbol->st_info) == STT_OBJECT)
							printf(symbol->st_shndx != SHN_UNDEF ? "V" : "v");
						else
						{
							printf(symbol->st_shndx != SHN_UNDEF ? "W" : "w");
						}
					}
					else if (symbol->st_shndx == SHN_ABS)
						printf("A");
					else if (symbol->st_shndx == SHN_COMMON)
						printf("C");
					else if (symbol->st_shndx == SHN_UNDEF)
						printf("U");
					else if (!strcmp((char*)shstrtab + (sht + symbol->st_shndx)->sh_name, ".debug")
							|| !strcmp((char*)shstrtab + (sht + symbol->st_shndx)->sh_name, ".line"))
						printf("N");
					else if ((sht + symbol->st_shndx)->sh_flags & SHF_EXECINSTR)
						printf(ELF64_ST_BIND(symbol->st_info) != STB_LOCAL ? "T" : "t");
					else if ((sht + symbol->st_shndx)->sh_flags & SHF_WRITE
							&& (sht + symbol->st_shndx)->sh_flags & SHF_ALLOC)
					{
						if ((sht + symbol->st_shndx)->sh_type == SHT_NOBITS)
							printf(ELF64_ST_BIND(symbol->st_info) != STB_LOCAL ? "B" : "b");
						else
							printf(ELF64_ST_BIND(symbol->st_info) != STB_LOCAL ? "D" : "d");
					}
					else if (!((sht + symbol->st_shndx)->sh_flags & SHF_WRITE))
						printf(ELF64_ST_BIND(symbol->st_info) != STB_LOCAL ? "R" : "r");
					else
						printf("?");
					printf(" ");

					// print name
					if (symbol->st_name)
						printf("%s", strtab + symbol->st_name);
					else
						printf("(null)");
					printf("\n");

				}
			}
		}
	}
}

static int handle_file(char const* filename)
{
	int fd;
	struct stat finfo;
	void* fmap;
	unsigned char class;

	fd = open(filename, O_RDONLY);
	if (fd == -1 || fstat(fd, &finfo) == -1)
		return 0;
	fmap = mmap(NULL, finfo.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
	close(fd);
	if (fmap == MAP_FAILED)
		return 0;
	class = check_intpr(fmap, finfo.st_size);
	if (class == ELFCLASS32 && finfo.st_size >= 52)
	{
		handle_32bits(fmap);
	}
	else if (class == ELFCLASS64 && finfo.st_size >= 64)
	{
		handle_64bits(fmap);
	}
	else
		// TODO: munmap if error
		return 0;
	munmap(fmap, finfo.st_size);
	return 1;
}

int main(int argc, char const* const* argv)
{
	if (argc == 1)
	{
		if (!handle_file("a.out"))
			perror("nm");
	}
	else
	{
		for (int i = 1; i < argc; i++)
		{
			if (!handle_file(*(argv + i)))
				perror("nm");
		}
	}
	return 0;
}
