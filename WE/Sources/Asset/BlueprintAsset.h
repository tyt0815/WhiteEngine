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
	enum class EPropertyType : int
	{
		EPT_Float = 0,
		EPT_Boolean,
		EPT_String,
		EPT_Float3,
		EPT_StringArray,
		EPT_TypeNum
	};

	struct FProperty
	{
		using FPropertyValue = std::variant<
			float,
			XMFLOAT3,
			bool,
			std::string,
			std::vector<std::string>
		>;

		std::string Name;
		EPropertyType Type;
		FPropertyValue Value;
	};

	enum class EFunctionParameterType : int
	{
		EFPT_Function = 0,
		EFPT_Property,
		EFPT_ConstValue
	};

	struct FFunctionNode
	{
		// 호출되는 함수 이름
		std::string Call;

		// 함수를 호출할 객체
		std::string Target;

		// 함수의 리턴이 반환될 WProperty 이름
		std::string Name;

		TArray<FProperty> StaticParameters;

		TArray<FProperty> PropertyParameters;

		TArray<TSharedPtr<FFunctionNode>> FunctionParameters;
	};

	struct FEventNode
	{
		std::string Name;
		TArray<TSharedPtr<FFunctionNode>> Functions;
	};

	struct FComponentNode
	{
		std::string ParentClass;

		TArray<FProperty> Properties;

		TArray<FProperty> Variables;

		TArray<FEventNode> Events;
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

		TArray<FProperty> Variables;

		TArray<TSharedPtr<FAttachedComponentNode>> AttachedComponents;

		TArray<FEventNode> Events;
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

	void SerializeProperty(FXMLElement* PropertyElement, FBinaryWriter& Writer);

	void DeserializeProperties(TArray<BlueprintAsset::FProperty>& Properties, FBinaryReader& Reader);

	void DeserializeProperty(BlueprintAsset::FProperty& Property, FBinaryReader& Reader);

	void SerializeComponent(FXMLElement* ComponentElement, FBinaryWriter& Writer);

	void DeserializeComponent(FBinaryReader& Reader, BlueprintAsset::FComponentNode* CompNode);

	void SerializeEvents(FXMLElement* EventsElement, FBinaryWriter& Writer);

	void SerializeEvent(FXMLElement* EventElement, FBinaryWriter& Writer);

	void SerializeFunction(FXMLElement* FuncElement, FBinaryWriter& Writer);

	void DeserializeEvents(FBinaryReader& Reader, TArray<BlueprintAsset::FEventNode>& EventsNode);

	void DeserializeEvent(FBinaryReader& Reader, BlueprintAsset::FEventNode* EventNode);

	void DeserializeFunction(FBinaryReader& Reader, BlueprintAsset::FFunctionNode* FuncNode);

private:
	virtual bool LoadAsset(const std::wstring& FilePath) override final;

	bool SmartLoad(const std::wstring& FilePath, TArray<unsigned char>& RawBuffer);

	bool CheckIfNeedCompile(const std::wstring& Src, const std::wstring& Bin);
};

class FActorBlueprintAsset : public FBlueprintAsset
{
	typedef FBlueprintAsset Super;
protected:
	virtual void Serialize(FXMLElement* RootElement, FBinaryWriter& Writer) override;

	virtual void Deserialize(FBinaryReader& Reader) override;

	void SerializeAttachedComponents(FXMLElement* AttachedComponentsElement, FBinaryWriter& Writer);

	void DeserializeAttachedComponents(FBinaryReader& Reader);

	void DeserializeAttachedComponent(BlueprintAsset::FAttachedComponentNode* AttachedCompNode, FBinaryReader& Reader);

	virtual void RegisterToFactory() override;

private:
	BlueprintAsset::FActorNode mRootNode;
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