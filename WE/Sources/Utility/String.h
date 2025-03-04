#pragma once

#include <stringapiset.h>

inline std::wstring AnsiToWString(const std::string& str)
{
    wchar_t buffer[512];
    MultiByteToWideChar(CP_ACP, 0, str.c_str(), -1, buffer, 512);
    return std::wstring(buffer);
}