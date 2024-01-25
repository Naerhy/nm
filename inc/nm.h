#ifndef NM_H
#define NM_H

/*
 * check if size of file is >= 5
 * verify ELF header + 32 or 64 bits
 * if valid -> verify if size >= structure size
 * parse structure (if needed)
 * loop on symbols?
 *
 * remove all symbols which are st_info = FILE or st_shndx = ABS
 */

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

#endif