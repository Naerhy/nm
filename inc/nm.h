#ifndef NM_H
#define NM_H

// TODO: print (null) when symbol name is full of 0000 [?]

#include <elf.h>
#include <errno.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

// TODO: remove
#include <string.h>

extern int same_endian;

unsigned char parse_intpr(unsigned char const* intpr, off_t fsize);
int parse_endian(unsigned char const* intpr);
int parse_header32(Elf32_Ehdr* header, off_t fsize);
int parse_header64(Elf64_Ehdr* header, off_t fsize);

void print_symbols32(Elf32_Ehdr* header);
void print_symbols64(Elf64_Ehdr* header);

uint16_t sw16(uint16_t v);
uint32_t sw32(uint32_t v);
uint64_t sw64(uint64_t v);

#endif
