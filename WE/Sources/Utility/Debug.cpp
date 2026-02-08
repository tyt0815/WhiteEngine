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
