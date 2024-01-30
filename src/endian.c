#include "nm.h"

uint16_t sw16(uint16_t v)
{
	if (same_endian)
		return v;
	return (v << 8) | (v >> 8);
}

uint32_t sw32(uint32_t v)
{
	if (same_endian)
		return v;
	return ((v << 24) & 0xFF000000)
			| ((v << 8) & 0x00FF0000)
			| ((v >> 8) & 0x0000FF00)
			| ((v >> 24) & 0x000000FF);
}

uint64_t sw64(uint64_t v)
{
	if (same_endian)
		return v;
	return ((v << 56) & 0xFF00000000000000ULL)
			| ((v << 40) & 0x00FF000000000000ULL)
            | ((v << 24) & 0x0000FF0000000000ULL)
			| ((v << 8)  & 0x000000FF00000000ULL)
			| ((v >> 8)  & 0x00000000FF000000ULL)
			| ((v >> 24) & 0x0000000000FF0000ULL)
			| ((v >> 40) & 0x000000000000FF00ULL)
			| ((v >> 56) & 0x00000000000000FFULL);
}
