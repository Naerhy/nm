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
	unsigned char* shtname;
	Elf32_Shdr* symtab;
	unsigned char* strtab;
	Elf32_Sym* symbols;
	Elf32_Sym* curr_symbol;

	if (header->e_ehsize != 52 || !header->e_shoff)
		return ;
	sht = (Elf32_Shdr*)((unsigned char*)header + header->e_shoff);
	shtname = (unsigned char*)header + (sht + header->e_shstrndx)->sh_offset;
	for (uint16_t i = 0; i < header->e_shnum; i++)
	{
		if (!strcmp(".symtab", (char*)shtname + (sht + i)->sh_name))
		{
			symtab = sht + i;
			strtab = (unsigned char*)header + (sht + symtab->sh_link)->sh_offset;
			symbols = (Elf32_Sym*)((unsigned char*)header + symtab->sh_offset);
			for (uint64_t j = 0; j < symtab->sh_size; j += symtab->sh_entsize)
			{
				curr_symbol = symbols + j / symtab->sh_entsize;
				if (curr_symbol->st_info != STT_FILE && curr_symbol->st_name)
					printf("%s\n", strtab + curr_symbol->st_name);
			}
		}
	}
}

static void handle_64bits(Elf64_Ehdr* header)
{
	Elf64_Shdr* sht;
	unsigned char* shtname;
	Elf64_Shdr* symtab;
	unsigned char* strtab;
	Elf64_Sym* symbols;
	Elf64_Sym* curr_symbol;

	if (header->e_ehsize != 64 || !header->e_shoff)
		return ;
	sht = (Elf64_Shdr*)((unsigned char*)header + header->e_shoff);
	shtname = (unsigned char*)header + (sht + header->e_shstrndx)->sh_offset;
	for (uint16_t i = 0; i < header->e_shnum; i++)
	{
		if (!strcmp(".symtab", (char*)shtname + (sht + i)->sh_name))
		{
			symtab = sht + i;
			strtab = (unsigned char*)header + (sht + symtab->sh_link)->sh_offset;
			symbols = (Elf64_Sym*)((unsigned char*)header + symtab->sh_offset);
			for (uint64_t j = 0; j < symtab->sh_size; j += symtab->sh_entsize)
			{
				curr_symbol = symbols + j / symtab->sh_entsize;
				if (curr_symbol->st_info != STT_FILE && curr_symbol->st_name)
					printf("%s\n", strtab + curr_symbol->st_name);
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
