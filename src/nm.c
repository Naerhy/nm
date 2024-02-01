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
	int save;

	fd = open(filename, O_RDONLY);
	if (fd == -1 || fstat(fd, &finfo) == -1)
	{
		wrerr(filename, strerror(errno));
		return 0;
	}
	if (S_ISDIR(finfo.st_mode))
	{
		close(fd);
		wrerr(filename, strerror(EISDIR));
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
		munmap(fmap, finfo.st_size);
		wrerr(filename, "File format not recognized");
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
	if (class == ELFCLASS32 && (size_t)finfo.st_size >= sizeof(Elf32_Ehdr))
	{
		if (!parse_header32(fmap, finfo.st_size))
		{
			wrerr(filename, "Corrupted ELF header");
			munmap(fmap, finfo.st_size);
			return 0;
		}
		save = save_symbols32(fmap, nm);
		if (save < 1)
		{
			wrerr(filename, save == -1 ? "No symbols" : strerror(errno));
			munmap(fmap, finfo.st_size);
			return 0;
		}
		print_symbols(nm, filename, 32);
	}
	else if (class == ELFCLASS64 && (size_t)finfo.st_size >= sizeof(Elf64_Ehdr))
	{
		if (!parse_header64(fmap, finfo.st_size))
		{
			wrerr(filename, "Corrupted ELF header");
			munmap(fmap, finfo.st_size);
			return 0;
		}
		save = save_symbols64(fmap, nm);
		if (save < 1)
		{
			wrerr(filename, save == -1 ? "No symbols" : strerror(errno));
			munmap(fmap, finfo.st_size);
			return 0;
		}
		print_symbols(nm, filename, 64);
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

static void free_symbols(void* content)
{
	Symbol* symbol;

	symbol = (Symbol*)content;
	free(symbol->value);
	free(symbol->name);
	free(symbol);
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
		free(nm.filenames);
		wrerr(NULL, "Invalid option");
		return EXIT_FAILURE;
	}
	if (!nm.nbfiles)
	{
		if (!handle_file(&nm, "a.out"))
			retcode = EXIT_FAILURE;
		lklist_clear(&nm.symbols, free_symbols);
	}
	else
	{
		for (size_t i = 0; i < nm.nbfiles; i++)
		{
			if (!handle_file(&nm, nm.filenames[i]))
				retcode = EXIT_FAILURE;
			lklist_clear(&nm.symbols, free_symbols);
		}
	}
	free(nm.filenames);
	return retcode;
}
