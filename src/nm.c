#include "nm.h"

int same_endian = -1;

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
	class = parse_intpr(fmap, finfo.st_size);
	same_endian = parse_endian(fmap);
	if (same_endian == -1)
	{
		munmap(fmap, finfo.st_size);
		return 0;
	}
	if (class == ELFCLASS32 && (size_t)finfo.st_size >= sizeof(Elf32_Ehdr))
	{
		if (!parse_header32(fmap, finfo.st_size))
		{
			munmap(fmap, finfo.st_size);
			return 0;
		}
		print_symbols32(fmap);
	}
	else if (class == ELFCLASS64 && (size_t)finfo.st_size >= sizeof(Elf64_Ehdr))
	{
		if (!parse_header64(fmap, finfo.st_size))
		{
			munmap(fmap, finfo.st_size);
			return 0;
		}
		print_symbols64(fmap);
	}
	else
	{
		munmap(fmap, finfo.st_size);
		return 0;
	}
	munmap(fmap, finfo.st_size);
	return 1;
}

int main(int argc, char const* const* argv)
{
	// TODO: print filename before nm output if multiple args, do not print if only 1 arg
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
