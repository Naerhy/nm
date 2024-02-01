#include "nm.h"

void wrchar(char c)
{
	write(STDOUT_FILENO, &c, 1);
}

void wrstr(char const* str)
{
	write(STDOUT_FILENO, str, ft_strlen(str));
}

void wrerr(char const* filename, char const* errmsg)
{
	if (filename)
	{
		wrstr("nm(");
		wrstr(filename);
		wrstr("): ");
	}
	else
		wrstr("nm: ");
	wrstr(errmsg);
	wrchar('\n');
}

void write_value(char const* value, int bits)
{
	size_t maxlen;

	if (value)
	{
		maxlen = (bits == 64) ? 16 : 8;
		for (size_t i = 0; i < maxlen - ft_strlen(value); i++)
			wrchar('0');
		wrstr(value);
	}
	else
		wrstr(bits == 64 ? "                " : "        ");
}
