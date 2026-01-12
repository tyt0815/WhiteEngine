#pragma once
#include <string>
#include <fstream>
#include <nlohmann/json.hpp>

using FJson = nlohmann::json;

namespace Asset
{
	bool OpenFile(const std::wstring& FilePath, std::ifstream& File);

	bool LoadJSON(const std::wstring& FilePath, FJson& Json);
};