#include "nm.h"

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
