#pragma once

#include <Windows.h>
#include <string>

inline std::wstring AnsiToWString(const std::string& str)
{
    wchar_t buffer[512];
    MultiByteToWideChar(CP_ACP, 0, str.c_str(), -1, buffer, 512);
    return std::wstring(buffer);
}

inline std::string WStringToString(const std::wstring& wstr) 
{
    if (wstr.empty()) return {};

    int size_needed = WideCharToMultiByte(CP_UTF8, 0,
        wstr.c_str(), (int)wstr.size(), nullptr, 0, nullptr, nullptr);

    std::string result(size_needed, 0);
    WideCharToMultiByte(CP_UTF8, 0,
        wstr.c_str(), (int)wstr.size(), &result[0], size_needed, nullptr, nullptr);

    return result;
}