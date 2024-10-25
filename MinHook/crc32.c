#include "crc32.h"

static DWORD g_table[256];

VOID CRC32Intialize()
{
	DWORD polynomial = 0xEDB88320;
	for (int i = 0; i < 256; i++) {
		DWORD c = i;
		for (int j = 0; j < 8; j++) {
			if (c & 1)  c = polynomial ^ (c >> 1);
			else c >>= 1;
		}
		g_table[i] = c;
	}
}

DWORD CRC32Update(DWORD initial, LPCBYTE buf, DWORD size)
{
	if (buf == NULL || size == 0) return initial;

	DWORD c = initial ^ 0xFFFFFFFF;
	for (int i = 0; i < size; ++i)
		c = g_table[(c ^ buf[i]) & 0xFF] ^ (c >> 8);
	return c ^ 0xFFFFFFFF;
}
