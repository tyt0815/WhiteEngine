#pragma once

#include "Asset.h"
#include "Utility/Container.h"
#include "Utility/Memory.h"
#include <tinyxml2.h>
#include <variant>

using FXMLDocument = tinyxml2::XMLDocument;
using FXMLElement = tinyxml2::XMLElement;
using FXMLAttribute = tinyxml2::XMLAttribute;

class FBinaryWriter;
class FBinaryReader;

namespace BlueprintAsset
{
	enum class EPropertyType : std::uint8_t
	{
		EPT_Float = 0,
		EPT_Boolean,
		EPT_Raw,
		EPT_TypeNum
	};

	struct FProperty
	{
		using FPropertyValue = std::variant<
			float,
			bool,
			std::string
		>;

		std::string Name;
		EPropertyType Type;
		FPropertyValue Value;
	};

	struct FInitializer
	{
		std::string Name;
		std::string Value;
	};

	struct FComponentNode
	{
		std::string Name;
		std::string Class;
		TArray<FProperty> Properties;
		TArray<FInitializer> Initializers;
	};

	struct FActorNode
	{
		std::string ParentClass;

		TArray<FProperty> Properties; 

		TArray<TSharedPtr<FComponentNode>> Components;

		TArray<FInitializer> Initializers;
	};
}

class FBlueprintAsset : public FAsset
{
public:
	BlueprintAsset::FActorNode mActorNode;

private:
	virtual bool LoadAsset(const std::wstring& FilePath) override;

	void DeserializeProperties(TArray<BlueprintAsset::FProperty>& Properties, FBinaryReader& Reader);

	void DeserializeInitializers(TArray<BlueprintAsset::FInitializer>& Initializers, FBinaryReader& Reader);

	void DeserializeComponents(FBinaryReader& Reader);

	void DeserializeComponent(BlueprintAsset::FComponentNode* CompNode, FBinaryReader& Reader);
};

class FBlueprintAssetCompiler : public FAssetCompiler
{
	virtual bool OnCompile(const std::wstring& SrcPath, std::vector<unsigned char>& OutBuffer) override;

	void SerializeProperties(FXMLElement* PropertiesElement, FBinaryWriter& Writer);

	void SerializeInitializer(FXMLElement* InitializersElement, FBinaryWriter& Writer);

	void SerializeComponents(FXMLElement* ComponentsElement, FBinaryWriter& Writer);
};