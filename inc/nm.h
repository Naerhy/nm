#ifndef NM_H
#define NM_H

// TODO: print (null) when symbol name is full of 0000 [?]
// check if no segfault when trying to read symbol->value when sorting

// symbols sorting:
// - alphabetical sort but priority for __ in front
//   - if cmp == 0 -> sort by value

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

typedef struct LkList
{
	void* content;
	struct LkList* next;
} LkList;

typedef struct Nm
{
	LkList* symbols;
	uint8_t flags;
	char** filenames;
	size_t nbfiles;
} Nm;

extern int endianness;

int parse_args(Nm* nm, int argc, char** argv);
unsigned char parse_intpr(unsigned char const* intpr, off_t fsize);
int parse_endian(unsigned char const* intpr);
int parse_header32(Elf32_Ehdr* header, off_t fsize);
int parse_header64(Elf64_Ehdr* header, off_t fsize);

int save_symbols32(Elf32_Ehdr* header, Nm* nm);
int save_symbols64(Elf64_Ehdr* header, Nm* nm);

void sort_list(LkList* symbols);

uint16_t sw16(uint16_t v);
uint32_t sw32(uint32_t v);
uint64_t sw64(uint64_t v);

size_t ft_strlen(char const* s);
char* ft_strcpy(char* dest, char const* src, size_t n);
char* ft_strdup(char const* s, size_t n);
int ft_strcmp(char const* s1, char const* s2, size_t n);
char* ft_itoa(Elf64_Addr value, char const* base);

void wrchar(char c);
void wrstr(char const* str);
void wrerr(char const* filename, char const* errmsg);
void write_value(char const* value, int bits);

LkList* lklist_create(void* content);
void lklist_add(LkList** head, LkList* node);
void lklist_delete(LkList** head, LkList* node, void (*delete)(void*));
void lklist_clear(LkList** head, void (*delete)(void*));
LkList* lklist_nth(LkList* head, size_t pos);
LkList* lklist_last(LkList* head);
size_t lklist_pos(LkList const* head, LkList const* node);

#endif
