#include "nm.h"

int endianness = -1;

static void init_nm(Nm* nm)
{
	nm->symbols = NULL;
	nm->nbsym = 0;
	nm->flags = 0x4;
	nm->filenames = NULL;
	nm->nbfiles = 0;
}

static void free_symbols(Symbol* symbols, size_t size)
{
	for (size_t i = 0; i < size; i++)
	{
		free(symbols[i].value);
		free(symbols[i].name);
	}
	free(symbols);
}

static void print_symbols(Nm* nm, char const* filename, int bits)
{
	if (nm->nbfiles > 1)
	{
		wrchar('\n');
		wrstr(filename);
		wrchar(':');
		wrchar('\n');
	}
	for (size_t i = 0; i < nm->nbsym; i++)
	{
		write_value(nm->symbols[i].value, bits);
		wrchar(' ');
		wrchar(nm->symbols[i].type);
		wrchar(' ');
		wrstr(nm->symbols[i].name);
		wrchar('\n');
	}
}

static int handle_file(Nm* nm, char const* filename)
{
	int fd;
	struct stat finfo;
	void* fmap;
	unsigned char class;
	int save;

	fd = open(filename, O_RDONLY);
	if (fd == -1 || fstat(fd, &finfo) == -1)
	{
		wrerr(filename, strerror(errno));
		return 0;
	}
	if (S_ISDIR(finfo.st_mode))
	{
		wrerr(filename, strerror(EISDIR));
		close(fd);
		return 0;
	}
	fmap = mmap(NULL, finfo.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
	close(fd);
	if (fmap == MAP_FAILED)
	{
		wrerr(filename, strerror(errno));
		return 0;
	}
	class = parse_intpr(fmap, finfo.st_size);
	if (class == ELFCLASSNONE)
	{
		wrerr(filename, "File format not recognized");
		munmap(fmap, finfo.st_size);
		return 0;
	}
	endianness = parse_endian(fmap);
	if (endianness == -1)
	{
		munmap(fmap, finfo.st_size);
		wrerr(filename, "Endianness not recognized");
		return 0;
	}
	nm->symbols = NULL;
	if ((class == ELFCLASS32 && (size_t)finfo.st_size >= sizeof(Elf32_Ehdr))
			|| (class == ELFCLASS64 && (size_t)finfo.st_size >= sizeof(Elf64_Ehdr)))
	{
		if (!parse_header(fmap, finfo.st_size, (class == ELFCLASS32) ? 32 : 64))
		{
			wrerr(filename, "Corrupted ELF header");
			munmap(fmap, finfo.st_size);
			return 0;
		}
		save = (class == ELFCLASS32) ? save_symbols32(fmap, nm, finfo.st_size) :
				save_symbols64(fmap, nm, finfo.st_size);
		if (save < 1)
		{
			wrerr(filename, (save == -2) ? "Corrupted ELF file" :
					(save == -1) ? "No symbols" : strerror(errno));
			free_symbols(nm->symbols, nm->nbsym);
			munmap(fmap, finfo.st_size);
			return 0;
		}
		if (nm->flags & SORT)
			sort_symbols(nm->symbols, nm->nbsym, nm->flags & REVERSE);
		print_symbols(nm, filename, (class == ELFCLASS32) ? 32 : 64);
		free_symbols(nm->symbols, nm->nbsym);
	}
	else
	{
		munmap(fmap, finfo.st_size);
		wrerr(filename, "Corrupted ELF header");
		return 0;
	}
	munmap(fmap, finfo.st_size);
	return 1;
}

int main(int argc, char** argv)
{
	Nm nm;
	int retcode;

	retcode = EXIT_SUCCESS;
	init_nm(&nm);
	nm.filenames = malloc(sizeof(char*) * argc);
	if (!nm.filenames)
	{
		wrerr(NULL, strerror(errno));
		return EXIT_FAILURE;
	}
	if (!parse_args(&nm, argc, argv))
	{
		wrerr(NULL, "Invalid option");
		free(nm.filenames);
		return EXIT_FAILURE;
	}
	if (!nm.nbfiles)
	{
		if (!handle_file(&nm, "a.out"))
			retcode = EXIT_FAILURE;
	}
	else
	{
		for (size_t i = 0; i < nm.nbfiles; i++)
		{
			if (!handle_file(&nm, nm.filenames[i]))
				retcode = EXIT_FAILURE;
		}
	}
	free(nm.filenames);
	return retcode;
}
