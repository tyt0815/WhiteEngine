#include "Debug.h"
#include <Windows.h>

void ShowMessageBox(std::wstring Content)
{
	MessageBox(0, Content.c_str(), 0, 0);
}
