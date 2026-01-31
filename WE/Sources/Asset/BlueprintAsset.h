#pragma once

#include "Asset.h"
#include "Utility/Container.h"
#include <tinyxml2.h>

using FXMLDocument = tinyxml2::XMLDocument;
using FXMLElement = tinyxml2::XMLElement;
using FXMLAttribute = tinyxml2::XMLAttribute;

class FBinaryWriter;
class FBinaryReader;

namespace BlueprintAsset
{

	struct FStaticMeshComponentInfo
	{
		std::string StaticMesh;
	};
}

class FBlueprintAsset : public FAsset
{
	
public:
	std::string mParentClass;

	std::unordered_map<std::string, float> mFloatMap;

	std::unordered_map<std::string, BlueprintAsset::FStaticMeshComponentInfo> mStaticMeshComponentMap;

private:
	virtual bool LoadAsset(const std::wstring& FilePath) override;

	void DeserializeProperties(FBinaryReader& Reader);

	void DeserializeComponents(FBinaryReader& Reader);
};

class FBlueprintAssetCompiler : public FAssetCompiler
{
	virtual bool OnCompile(const std::wstring& SrcPath, std::vector<unsigned char>& OutBuffer) override;

	void SerializeProperties(FXMLElement* PropertiesElement, FBinaryWriter& Writer);

	void SerializeComponents(FXMLElement* ComponentsElement, FBinaryWriter& Writer);
};