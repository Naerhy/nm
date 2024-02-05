#include "nm.h"

static int valuecmp(char const* s1, char const* s2)
{
	if (!s1)
		return -1;
	else if (!s2)
		return 1;
	else if (ft_atoi(s1) < ft_atoi(s2))
		return -1;
	else if (ft_atoi(s1) > ft_atoi(s2))
		return 1;
	else
		return 0;
}

static size_t pre_underscr(char const* s)
{
	size_t i;

	i = 0;
	while (s[i] && s[i] == '_')
		i++;
	return i;
}

static int namecmp(char const* s1, char const* s2)
{
	char c1;
	char c2;
	size_t pre_s1;
	size_t pre_s2;

	for (size_t i = 0, j = 0; s1[i] || s2[j]; i++, j++)
	{
		while (s1[i] && !ft_isalnum(s1[i]))
			i++;
		c1 = (s1[i] >= 'A' && s1[i] <= 'Z') ? s1[i] + 32 : s1[i];
		while (s2[j] && !ft_isalnum(s2[j]))
			j++;
		c2 = (s2[j] >= 'A' && s2[j] <= 'Z') ? s2[j] + 32 : s2[j];
		if (c1 != c2)
			return c1 - c2;
	}
	for (size_t i = 0, j = 0; s1[i] || s2[j]; i++, j++)
	{
		while (s1[i] && !ft_isalnum(s1[i]))
			i++;
		while (s2[j] && !ft_isalnum(s2[j]))
			j++;
		if (s1[i] != s2[j])
			return (s1[i] - s2[j]) * -1;
	}
	pre_s1 = pre_underscr(s1);
	pre_s2 = pre_underscr(s2);
	if (pre_s1 < pre_s2)
		return 1;
	else if (pre_s1 > pre_s2)
		return -1;
	else
		return ft_strcmp(s1 + pre_s1, s2 + pre_s2, 0);
}

void sort_symbols(Symbol* symbols, size_t size, int reverse)
{
	int cmp;
	Symbol tmp;
	int rev;

	rev = (reverse) ? -1 : 1;
	for (size_t i = 0; i < size; i++)
	{
		for (size_t j = i + 1; j < size; j++)
		{
			cmp = namecmp(symbols[i].name, symbols[j].name) * rev;
			if (cmp > 0)
			{
				tmp = symbols[i];
				symbols[i] = symbols[j];
				symbols[j] = tmp;
			}
			else if (!cmp && valuecmp(symbols[i].value, symbols[j].value) > 0)
			{
				tmp = symbols[i];
				symbols[i] = symbols[j];
				symbols[j] = tmp;
			}
			else { /* ... */ }
		}
	}
}
