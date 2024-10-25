#pragma once
#include <windows.h>
#include <string>
#include <locale>
#include <codecvt>

std::string Utf16ToUtf8(const wchar_t* wstr)
{
	std::string convertedString;
	if (!wstr || !*wstr)
		return convertedString;
	auto requiredSize = WideCharToMultiByte(CP_UTF8, 0, wstr, -1, nullptr, 0, nullptr, nullptr);
	if (requiredSize > 0) {
		convertedString.resize(requiredSize - 1);
		if (!WideCharToMultiByte(CP_UTF8, 0, wstr, -1, (char*)convertedString.c_str(), requiredSize, nullptr, nullptr))
			convertedString.clear();
	}
	return convertedString;
}

int PrintUtf16ToUtf8(char* buf, size_t size, const wchar_t* wstr)
{
	std::string utf8 = Utf16ToUtf8(wstr);
	return std::snprintf(buf, size, utf8.c_str());
}

std::wstring Utf8ToUtf16(const char* str)
{
	std::wstring convertedString;
	if (!str || !*str)
		return convertedString;
	int requiredSize = MultiByteToWideChar(CP_UTF8, 0, str, -1, nullptr, 0);
	if (requiredSize > 0) {
		convertedString.resize(requiredSize - 1);
		if (!MultiByteToWideChar(CP_UTF8, 0, str, -1, (wchar_t*)convertedString.c_str(), requiredSize))
			convertedString.clear();
	}
	return convertedString;
}

int PrintUtf8ToUtf16(wchar_t* buf, size_t size, const char* str)
{
	std::wstring utf16 = Utf8ToUtf16(str);
	return std::swprintf(buf, size, utf16.c_str());
}
