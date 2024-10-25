#pragma once
#include <iostream>
#include <cstdarg>
#include <debugapi.h>
#include "utf8.hpp"
#include "path.hpp"

// 1 打开调试输出.
#define ENABLE_DLOG 1

void dPrintfA(LPCSTR format, ...) {
#if ENABLE_DLOG
	static CHAR buffer[1024];
	memset(buffer, sizeof(buffer), 0);
	va_list args;
	va_start(args, format);
	std::vsnprintf(buffer, _countof(buffer), format, args);
	OutputDebugStringA(buffer);
#endif
}

void dPrintfW(LPCWSTR format, ...) {
#if ENABLE_DLOG
	static WCHAR buffer[1024];
	memset(buffer, sizeof(buffer), 0);
	va_list args;
	va_start(args, format);
	std::vswprintf(buffer, _countof(buffer), format, args);
	OutputDebugStringW(buffer);
#endif
}

#define dlog() dPrintfW(L"[%s] " __FUNCTION__ "\n", GetProcessBaseNameW())
#define dlogpa(fmt, ...) dPrintfA("[%s] " __FUNCTION__ " - " fmt "\n", GetProcessBaseNameA(), __VA_ARGS__)
#define dlogpw(fmt, ...) dPrintfW(L"[%s] " __FUNCTION__ " - " fmt "\n", GetProcessBaseNameW(), __VA_ARGS__)
#define dlogp dlogpa
