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

	struct FComponentNode
	{
		std::string ParentClass;

		TArray<FProperty> Properties;
	};

	struct FAttachedComponentNode
	{
		std::string Name;

		FComponentNode ComponentNode;
	};

	struct FActorNode
	{
		std::string ParentClass;

		TArray<FProperty> Properties; 

		TArray<TSharedPtr<FAttachedComponentNode>> AttachedComponents;
	};
}

class FBlueprintAsset : public FAsset
{
	typedef FAsset Super;

protected:
	bool OnCompile(const std::wstring& SrcPath, std::vector<unsigned char>& OutBuffer);

	virtual void RegisterToFactory() = 0;	

	virtual void Serialize(FXMLElement* RootElement, FBinaryWriter& Writer) = 0;

	virtual void Deserialize(FBinaryReader& Reader) = 0;

	void SerializeProperties(FXMLElement* PropertiesElement, FBinaryWriter& Writer);

	void DeserializeProperties(TArray<BlueprintAsset::FProperty>& Properties, FBinaryReader& Reader);

	void SerializeComponent(FXMLElement* ComponentsElement, FBinaryWriter& Writer);

	void DeserializeComponent(FBinaryReader& Reader, BlueprintAsset::FComponentNode* CompNode);

private:
	virtual bool LoadAsset(const std::wstring& FilePath) override final;

	bool SmartLoad(const std::wstring& FilePath, TArray<unsigned char>& RawBuffer);

	bool CheckIfNeedCompile(const std::wstring& Src, const std::wstring& Bin);
};

class FActorBlueprintAsset : public FBlueprintAsset
{
	typedef FBlueprintAsset Super;
public:
	BlueprintAsset::FActorNode mActorNode;

protected:
	virtual void Serialize(FXMLElement* RootElement, FBinaryWriter& Writer) override;

	virtual void Deserialize(FBinaryReader& Reader) override;

	void SerializeAttachedComponents(FXMLElement* ComponentsElement, FBinaryWriter& Writer);

	void DeserializeAttachedComponents(FBinaryReader& Reader);

	void DeserializeAttachedComponent(BlueprintAsset::FAttachedComponentNode* CompNode, FBinaryReader& Reader);

	virtual void RegisterToFactory() override;
};

class FComponentBlueprintAsset : public FBlueprintAsset
{
	typedef FBlueprintAsset Super;
protected:
	virtual void Serialize(FXMLElement* RootElement, FBinaryWriter& Writer) override;

	virtual void Deserialize(FBinaryReader& Reader) override;

	virtual void RegisterToFactory() override;

private:
	BlueprintAsset::FComponentNode mRootNode;
};