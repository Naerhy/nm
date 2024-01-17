#include "nm.h"

void handle_file(char const* filename)
{
	int fd;
	struct stat finfo;
	void* fmap;
	Elf64_Ehdr* header;

	fd = open(filename, O_RDONLY);
	if (fd == -1)
	{
		return ;
	}
	if (fstat(fd, &finfo) == -1)
	{
		return ;
	}
	fmap = mmap(NULL, finfo.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
	close(fd);
	if (fmap == MAP_FAILED)
	{
		return ;
	}
	printf("file %s of %zu bytes has been mapped\n", filename, finfo.st_size);
	header = (Elf64_Ehdr*)fmap;
	for (size_t i = 0; i < 4; i++)
		printf("%X\n", header->e_ident[i]);
	munmap(fmap, finfo.st_size);
}

int main(int argc, char const* const* argv)
{
	if (argc == 1)
	{
		handle_file("a.out");
	}
	else
	{
		for (int i = 1; i < argc; i++)
		{
			handle_file(*(argv + i));
		}
	}
	return 0;
}
