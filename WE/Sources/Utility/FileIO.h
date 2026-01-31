#pragma once
#include <string>
#include <fstream>
#include <vector>

void ReadFile(std::string FilePath, std::ifstream& FileStream);

namespace FileIO
{
	bool SaveBufferToFile(const std::wstring& Path, const std::vector<unsigned char>& Buffer);

	bool LoadBufferFromFile(const std::wstring& Path, std::vector<unsigned char>& OutBuffer);
}