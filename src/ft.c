#include "nm.h"

int ft_isalpha(int c)
{
	if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z'))
		return 1;
	return 0;
}

int ft_isdigit(int c)
{
	if (c >= '0' && c <= '9')
		return 1;
	return 0;
}

int ft_isalnum(int c)
{
	if (ft_isdigit(c) || ft_isalpha(c))
		return 1;
	return 0;
}

size_t ft_strlen(char const* s)
{
	size_t len;

	len = 0;
	while (s[len])
		len++;
	return len;
}

char* ft_strcpy(char* dest, char const* src, size_t n)
{
	size_t i;

	i = 0;
	while ((!n || i < n) && src[i])
	{
		dest[i] = src[i];
		i++;
	}
	dest[i] = '\0';
	return dest;
}

char* ft_strdup(char const* s, size_t n)
{
	char* dup;
	size_t len;

	len = n ? n : ft_strlen(s);
	dup = malloc(len + 1);
	if (!dup)
		return NULL;
	return ft_strcpy(dup, s, n);
}

int ft_strcmp(char const* s1, char const* s2, size_t n)
{
	for (size_t i = 0; (!n || i < n) && (s1[i] || s2[i]); i++)
	{
		if (s1[i] != s2[i])
			return s1[i] - s2[i];
	}
	return 0;
}

static size_t nblen(Elf64_Addr value)
{
	size_t len;

	len = 1;
	while (value >= 16)
	{
		len++;
		value /= 16;
	}
	return len;
}

char* ft_itoa(Elf64_Addr value, char const* base)
{
	char* str;
	size_t len;

	len = nblen(value);
	str = malloc(len + 1);
	if (!str)
		return NULL;
	str[len] = '\0';
	if (!value)
		str[0] = '0';
	while (value)
	{
		str[len - 1] = base[value % 16];
		value /= 16;
		len--;
	}
	return str;
}
