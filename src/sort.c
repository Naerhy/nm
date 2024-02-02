#include "nm.h"

static int valuecmp(char const* s1, char const* s2)
{
	if (!s1)
		return -1;
	if (!s2)
		return 1;
	return ft_strcmp(s1, s2, 0);
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
	size_t i;
	size_t j;
	char c1;
	char c2;
	size_t pre_s1;
	size_t pre_s2;

	i = 0;
	j = 0;
	while (s1[i] || s2[j])
	{
		while (s1[i] == '_' || s1[i] == '@')
			i++;
		c1 = (s1[i] >= 'A' && s1[i] <= 'Z') ? s1[i] + 32 : s1[i];
		while (s2[j] == '_' || s2[j] == '@')
			j++;
		c2 = (s2[j] >= 'A' && s2[j] <= 'Z') ? s2[j] + 32 : s2[j];
		if (c1 != c2)
			return c1 - c2;
		i++;
		j++;
	}

	i = 0;
	j = 0;
	while (s1[i] || s2[j])
	{
		while (s1[i] == '_')
			i++;
		while (s2[j] == '_')
			j++;
		if (s1[i] != s2[j])
			return (s1[i] - s2[j]) * -1;
		i++;
		j++;
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

void sort_list(LkList* symbols)
{
	LkList* bubble;
	int cmp;
	Symbol* sym1;
	Symbol* sym2;

	while (symbols)
	{
		bubble = symbols->next;
		while (bubble)
		{
			sym1 = symbols->content;
			sym2 = bubble->content;
			cmp = namecmp(sym1->name, sym2->name);
			if (cmp > 0)
			{
				symbols->content = bubble->content;
				bubble->content = sym1;
			}
			if (!cmp && valuecmp(sym1->value, sym2->value) > 0)
			{
				symbols->content = bubble->content;
				bubble->content = sym1;
			}
			bubble = bubble->next;
		}
		symbols = symbols->next;
	}
}
