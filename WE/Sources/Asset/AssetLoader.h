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

	bool LoadXML(const std::wstring& FilePath, tinyxml2::XMLDocument& Doc);

	bool LoadZlib(const std::wstring& FilePath, std::vector<unsigned char>& RawBuffer);

	bool LoadZLibXML(const std::wstring& FilePath, tinyxml2::XMLDocument& Doc);

	bool LoadLZ4(const std::wstring& FilePath, std::vector<unsigned char>& RawBuffer);
};