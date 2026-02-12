#include "Debug.h"
#include <Windows.h>

void ShowMessageBox(const std::string& Content)
{
	MessageBoxA(0, Content.c_str(), 0, 0);
}

void ShowMessageBox(const std::wstring& Content)
{
	MessageBoxW(0, Content.c_str(), 0, 0);
}

void Log(const std::string& Content)
{
	std::cout << Content << std::endl;
}

void Log(const std::wstring& Content)
{
	std::wcout << Content << std::endl;
}
