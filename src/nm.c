#include "nm.h"

int endianness = -1;

static void print_symbols(Nm* nm, char const* filename, int bits)
{
	LkList* tmp;
	Symbol* sym;

	if (nm->nbfiles > 1)
	{
		wrchar('\n');
		wrstr(filename);
		wrchar(':');
		wrchar('\n');
	}
	tmp = nm->symbols;
	while (tmp)
	{
		sym = tmp->content;
		write_value(sym->value, bits);
		wrchar(' ');
		wrchar(sym->type);
		wrchar(' ');
		wrstr(sym->name ? sym->name : "(null)");
		wrchar('\n');
		tmp = tmp->next;
	}
}

static int handle_file(Nm* nm, char const* filename)
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
	endianness = parse_endian(fmap);
	if (endianness == -1)
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
		save_symbols32(fmap, nm);
		print_symbols(nm, filename, 32);
	}
	else if (class == ELFCLASS64 && (size_t)finfo.st_size >= sizeof(Elf64_Ehdr))
	{
		if (!parse_header64(fmap, finfo.st_size))
		{
			munmap(fmap, finfo.st_size);
			return 0;
		}
		save_symbols64(fmap, nm);
		print_symbols(nm, filename, 64);
	}
	else
	{
		munmap(fmap, finfo.st_size);
		return 0;
	}
	munmap(fmap, finfo.st_size);
	return 1;
}

static void free_nm(Nm* nm)
{
	free(nm->filenames);
}

static void init_nm(Nm* nm)
{
	nm->symbols = NULL;
	nm->flags = 0x4;
	nm->filenames = NULL;
	nm->nbfiles = 0;
}

int main(int argc, char** argv)
{
	Nm nm;

	init_nm(&nm);
	nm.filenames = malloc(sizeof(char*) * argc);
	// TODO: improve this error
	if (!nm.filenames)
		return 1;
	parse_args(&nm, argc, argv);
	if (!nm.nbfiles)
	{
		// TODO: handle error
		if (!handle_file(&nm, "a.out"))
			return 1;
	}
	else
	{
		for (size_t i = 0; i < nm.nbfiles; i++)
		{
			// TODO: handle error
			if (!handle_file(&nm, nm.filenames[i]))
				return 1;
		}
	}
	free_nm(&nm);
	return 0;
}
