#pragma once
#include <windows.h>

VOID CRC32Intialize();
DWORD CRC32Update(DWORD initial, LPCBYTE buf, DWORD size);
