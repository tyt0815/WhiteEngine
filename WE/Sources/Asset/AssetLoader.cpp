#include "AssetLoader.h"
#include <sstream>
#include <Windows.h>

bool Asset::OpenFile(const std::wstring& FilePath, std::ifstream& File)
{
	if (File.is_open())
	{
		File.close();
	}

	File.open(FilePath);

	if (!File.is_open())
	{
		std::wstringstream ss;
		ss << L"파일을 열 수 없습니다." << FilePath << "\n";
		OutputDebugStringW(ss.str().c_str());
		return false;
	}

	return true;
}

bool Asset::LoadJSON(const std::wstring& FilePath, FJson& Json)
{
	std::ifstream File;

	if (OpenFile(FilePath, File))
	{
		File >> Json;

		return true;
	}

	return false;
}