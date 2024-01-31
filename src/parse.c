#include "nm.h"

unsigned char parse_intpr(unsigned char const* intpr, off_t fsize)
{
	if (fsize < EI_NIDENT || intpr[EI_MAG0] != ELFMAG0 || intpr[EI_MAG1] != ELFMAG1
			|| intpr[EI_MAG2] != ELFMAG2 || intpr[EI_MAG3] != ELFMAG3)
		return ELFCLASSNONE;
	return intpr[EI_CLASS];
}

int parse_endian(unsigned char const* intpr)
{
	unsigned int x;
	unsigned char* c;

	if (intpr[EI_DATA] != ELFDATA2LSB && intpr[EI_DATA] != ELFDATA2MSB)
		return -1;
	x = 1;
	c = (unsigned char*)&x;
	return (*c == 1 && intpr[EI_DATA] == ELFDATA2LSB) || (!*c && intpr[EI_DATA] == ELFDATA2MSB);

}

int parse_header32(Elf32_Ehdr* header, off_t fsize)
{
	uint16_t type;
	Elf32_Off phoff;
	Elf32_Off shoff;
	uint16_t phentsize;
	uint16_t shentsize;

	type = sw16(header->e_type);
	if (type != ET_REL && type != ET_EXEC && type != ET_DYN)
		return 0;
	phoff = sw32(header->e_phoff);
	shoff = sw32(header->e_shoff);
	if (phoff > (Elf32_Off)fsize || shoff > (Elf32_Off)fsize)
		return 0;
	if (sw16(header->e_ehsize) != sizeof(Elf32_Ehdr))
		return 0;
	phentsize = sw16(header->e_phentsize);
	shentsize = sw16(header->e_shentsize);
	if (phentsize != sizeof(Elf32_Phdr) || shentsize != sizeof(Elf32_Shdr))
		return 0;
	if (!sw16(header->e_shnum))
		return 0;
	if (phoff + phentsize * sw16(header->e_phnum) > (Elf32_Off)fsize
			|| shoff + shentsize * sw16(header->e_shnum) > (Elf32_Off)fsize)
		return 0;
	return 1;
}

int parse_header64(Elf64_Ehdr* header, off_t fsize)
{
	uint16_t type;
	Elf64_Off phoff;
	Elf64_Off shoff;
	uint16_t phentsize;
	uint16_t shentsize;

	type = sw16(header->e_type);
	if (type != ET_REL && type != ET_EXEC && type != ET_DYN)
		return 0;
	phoff = sw64(header->e_phoff);
	shoff = sw64(header->e_shoff);
	if (phoff > (Elf64_Off)fsize || shoff > (Elf64_Off)fsize)
		return 0;
	if (sw16(header->e_ehsize) != sizeof(Elf64_Ehdr))
		return 0;
	phentsize = sw16(header->e_phentsize);
	shentsize = sw16(header->e_shentsize);
	if (phentsize != sizeof(Elf64_Phdr) || shentsize != sizeof(Elf64_Shdr))
		return 0;
	if (!sw16(header->e_shnum))
		return 0;
	if (phoff + phentsize * sw16(header->e_phnum) > (Elf64_Off)fsize
			|| shoff + shentsize * sw16(header->e_shnum) > (Elf64_Off)fsize)
		return 0;
	return 1;
}
