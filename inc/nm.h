#ifndef NM_H
#define NM_H

#include <elf.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

#define DEBUG (1 << 0)
#define EXTERN (1 << 1)
#define SORT (1 << 2)
#define REVERSE (1 << 3)
#define UNDEFINED (1 << 4)

typedef struct Symbol
{
	char* value;
	char type;
	char* name;
} Symbol;

typedef struct Nm
{
	Symbol* symbols;
	size_t nbsym;
	uint8_t flags;
	char** filenames;
	size_t nbfiles;
} Nm;

extern int endianness;

int parse_args(Nm* nm, int argc, char** argv);
unsigned char parse_intpr(unsigned char const* intpr, off_t fsize);
int parse_endian(unsigned char const* intpr);
int parse_header(void* ehdr, off_t fsize, int bits);

int save_symbols32(Elf32_Ehdr* header, Nm* nm, off_t fsize);
int save_symbols64(Elf64_Ehdr* header, Nm* nm, off_t fsize);

void sort_symbols(Symbol* symbols, size_t size);

uint16_t sw16(uint16_t v);
uint32_t sw32(uint32_t v);
uint64_t sw64(uint64_t v);

int ft_isalpha(int c);
int ft_isdigit(int c);
int ft_isalnum(int c);
size_t ft_strlen(char const* s);
char* ft_strcpy(char* dest, char const* src, size_t n);
char* ft_strdup(char const* s, size_t n);
int ft_strcmp(char const* s1, char const* s2, size_t n);
char* ft_itoa(Elf64_Addr value, char const* base);
Elf64_Addr ft_atoi(char const* s);

void wrchar(char c);
void wrstr(char const* str);
void wrerr(char const* filename, char const* errmsg);
void write_value(char const* value, int bits);

#endif
