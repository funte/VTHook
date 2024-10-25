#pragma once
#include <iostream>
#include <string>
#include <algorithm>
#include <filesystem>
namespace fs = std::filesystem;
#include "utf8.hpp"

// 提取文件名称.
std::wstring GetFileNameW(std::wstring path)
{
	WCHAR buffer[MAX_PATH] = { L'\0' };
	if (path.length() > 0) {
		std::swprintf(buffer, _countof(buffer), path.c_str());

		auto pLastSeparator = std::wcsrchr(buffer, L'\\');
		if (pLastSeparator) {
			pLastSeparator++;
			memmove(buffer, pLastSeparator, (std::wcslen(pLastSeparator) + 1) * 2);
		}
		pLastSeparator = std::wcsrchr(buffer, L'/');
		if (pLastSeparator) {
			pLastSeparator++;
			memmove(buffer, pLastSeparator, (std::wcslen(pLastSeparator) + 1) * 2);
		}
	}
	return buffer;
}

// 提取文件名称, 不包含后缀.
std::wstring GetBaseNameW(std::wstring path)
{
	WCHAR buffer[MAX_PATH] = { L'\0' };
	if (path.length() > 0) {
		std::swprintf(buffer, _countof(buffer), path.c_str());

		auto pLastSeparator = std::wcsrchr(buffer, L'\\');
		if (pLastSeparator) {
			pLastSeparator++;
			memmove(buffer, pLastSeparator, (std::wcslen(pLastSeparator) + 1) * 2);
		}
		pLastSeparator = std::wcsrchr(buffer, L'/');
		if (pLastSeparator) {
			pLastSeparator++;
			memmove(buffer, pLastSeparator, (std::wcslen(pLastSeparator) + 1) * 2);
		}

		auto period = std::wcsrchr(buffer, L'.');
		if (period) *period = L'\0';
	}
	return buffer;
}


// 提取文件名称, 不包含后缀.
std::string GetFileNameA(std::string path)
{
	CHAR buffer[MAX_PATH];
	if (path.length() > 0) {
		std::snprintf(buffer, MAX_PATH, path.c_str());

		auto pLastSeparator = std::strrchr(buffer, '\\');
		if (pLastSeparator) {
			pLastSeparator++;
			memmove((void*)buffer, pLastSeparator, std::strlen(pLastSeparator) + 1);
		}
		pLastSeparator = std::strrchr(buffer, '/');
		if (pLastSeparator) {
			pLastSeparator++;
			memmove((void*)buffer, pLastSeparator, std::strlen(pLastSeparator) + 1);
		}
	}
	return buffer;
}

// 提取文件名称, 不包含后缀.
std::string GetBaseNameA(std::string path)
{
	CHAR buffer[MAX_PATH];
	if (path.length() > 0) {
		std::snprintf(buffer, MAX_PATH, path.c_str());

		auto pLastSeparator = std::strrchr(buffer, '\\');
		if (pLastSeparator) {
			pLastSeparator++;
			memmove((void*)buffer, pLastSeparator, std::strlen(pLastSeparator) + 1);
		}
		pLastSeparator = std::strrchr(buffer, '/');
		if (pLastSeparator) {
			pLastSeparator++;
			memmove((void*)buffer, pLastSeparator, std::strlen(pLastSeparator) + 1);
		}

		auto period = (char*)std::strrchr(buffer, '.');
		if (period) *period = '\0';
	}
	return buffer;
}

// 查询当前进程名称.
LPCWSTR GetProcessFileNameW()
{
	static WCHAR buffer[MAX_PATH] = { L'\0' };
	if (*buffer == L'\0') {
		GetModuleFileNameW(NULL, buffer, _countof(buffer));
		auto filename = GetFileNameW(buffer);
		std::swprintf(buffer, _countof(buffer), filename.c_str());
	}
	return buffer;
}

// 查询当前进程名称, 不包含后缀.
LPCWSTR GetProcessBaseNameW()
{
	static WCHAR buffer[MAX_PATH] = { L'\0' };
	if (*buffer == L'\0') {
		GetModuleFileNameW(NULL, buffer, _countof(buffer));
		auto basename = GetBaseNameW(buffer);
		std::swprintf(buffer, _countof(buffer), basename.c_str());
	}
	return buffer;
}

// 查询当前进程名称.
LPCSTR GetProcessFileNameA()
{
	static CHAR buffer[MAX_PATH] = { '\0' };
	if (*buffer == '\0') {
		PrintUtf16ToUtf8(buffer, MAX_PATH, GetProcessFileNameW());
	}
	return buffer;
}

// 查询当前进程名称, 不包含后缀.
LPCSTR GetProcessBaseNameA()
{
	static CHAR buffer[MAX_PATH] = { '\0' };
	if (*buffer == '\0') {
		PrintUtf16ToUtf8(buffer, MAX_PATH, GetProcessBaseNameW());
	}
	return buffer;
}
