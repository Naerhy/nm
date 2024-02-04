#include "nm.h"

int parse_args(Nm* nm, int argc, char** argv)
{
	for (int i = 1; i < argc; i++)
	{
		if (!ft_strcmp(argv[i], "-a", 0))
			nm->flags |= DEBUG;
		else if (!ft_strcmp(argv[i], "-g", 0))
			nm->flags |= EXTERN;
		else if (!ft_strcmp(argv[i], "-p", 0))
			nm->flags &= ~SORT;
		else if (!ft_strcmp(argv[i], "-r", 0))
			nm->flags |= REVERSE;
		else if (!ft_strcmp(argv[i], "-u", 0))
			nm->flags |= UNDEFINED;
		else if (ft_strlen(argv[i]) > 1 && argv[i][0] == '-')
			return 0;
		else
		{
			nm->filenames[nm->nbfiles] = argv[i];
			nm->nbfiles++;
		}
	}
	return 1;
}

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

static int parse_header32(Elf32_Ehdr* ehdr, off_t fsize)
{
	uint16_t type;
	Elf32_Off shoff;
	uint16_t shentsize;
	uint16_t shnum;

	type = sw16(ehdr->e_type);
	if (type != ET_REL && type != ET_EXEC && type != ET_DYN)
		return 0;
	if (sw16(ehdr->e_ehsize) != sizeof(Elf32_Ehdr))
		return 0;
	shoff = sw32(ehdr->e_shoff);
	shentsize = sw16(ehdr->e_shentsize);
	shnum = sw16(ehdr->e_shnum);
	if (!shoff || shentsize != sizeof(Elf32_Shdr) || !shnum ||
			shoff + shentsize * shnum > (uint32_t)fsize)
		return 0;
	return 1;
}

static int parse_header64(Elf64_Ehdr* ehdr, off_t fsize)
{
	uint16_t type;
	Elf64_Off shoff;
	uint16_t shentsize;
	uint16_t shnum;

	type = sw16(ehdr->e_type);
	if (type != ET_REL && type != ET_EXEC && type != ET_DYN)
		return 0;
	if (sw16(ehdr->e_ehsize) != sizeof(Elf64_Ehdr))
		return 0;
	shoff = sw64(ehdr->e_shoff);
	shentsize = sw16(ehdr->e_shentsize);
	shnum = sw16(ehdr->e_shnum);
	if (!shoff || shentsize != sizeof(Elf64_Shdr) || !shnum ||
			shoff + shentsize * shnum > (uint64_t)fsize)
		return 0;
	return 1;
}

int parse_header(void* ehdr, off_t fsize, int bits)
{
	return (bits == 32) ? parse_header32(ehdr, fsize) : parse_header64(ehdr, fsize);
}
