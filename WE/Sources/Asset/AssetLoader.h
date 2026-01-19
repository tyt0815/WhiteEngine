#pragma once
#include <string>
#include <fstream>
#include <nlohmann/json.hpp>
#include <tinyxml2.h>

using FJson = nlohmann::json;

namespace Asset
{
	bool OpenFile(const std::wstring& FilePath, std::ifstream& File);

	bool LoadJSON(const std::wstring& FilePath, FJson& Json);

	bool LoadXML(const std::string& FilePath, tinyxml2::XMLDocument& Doc);
};