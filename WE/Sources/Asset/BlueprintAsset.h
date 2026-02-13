#pragma once

#include "Asset.h"
#include "Utility/Container.h"
#include "Utility/Memory.h"
#include "BlueprintTypes.h"
#include <tinyxml2.h>
#include <variant>

using FXMLDocument = tinyxml2::XMLDocument;
using FXMLElement = tinyxml2::XMLElement;
using FXMLAttribute = tinyxml2::XMLAttribute;

class FBinaryWriter;
class FBinaryReader;

struct FAttribute
{
	std::string Name;
	std::unordered_map<std::string, std::string> Attributes;
};

struct FBlueprintComponentNode
{
	std::string Type;
	WAttributesMap Attributes;

	TArray<TUniquePtr<FBlueprintComponentNode>> AttachedComponents;
};

struct FBlueprintActionNode
{
	std::string Name;
	WAttributesMap Attributes;
};

struct FBlueprintEventNode
{
	std::string Name;
	WAttributesMap Attributes;
	TArray<TUniquePtr<FBlueprintActionNode>> Actions;
};

struct FBlueprintState
{
	std::string Name;
	TArray<TUniquePtr<FBlueprintEventNode>> Events;
};

struct FBlueprintStateMachine
{
	std::string InitialState;
	TArray<TUniquePtr<FBlueprintState>> States;
};

class FBlueprintAsset : public FAsset
{
	typedef FAsset Super;

public:
	std::string mParentClass;

	std::unordered_map<std::string, WAttributesMap> mConfigs;

	TArray<TUniquePtr<FBlueprintComponentNode>> mAttachedComponents;
	

protected:
	virtual bool LoadAsset(const std::wstring& FilePath) override;

private:
	bool SmartLoad(const std::wstring& FilePath, TArray<unsigned char>& RawBuffer);

	void RegisterToFactory();

	bool CheckIfNeedCompile(const std::wstring& Src, const std::wstring& Bin);

	bool OnCompile(const std::wstring& SrcPath, std::vector<unsigned char>& OutBuffer);

	void Serialize(FBinaryWriter& Writer, FXMLElement* RootElement);

	void Deserialize(FBinaryReader& Reader);

	void SerializeAttributes(FBinaryWriter& Writer, FXMLElement* Element);

	void DeserializeAttributes(FBinaryReader& Reader, WAttributesMap& AttributesMap);

	void SerializeConfigs(FBinaryWriter& Writer, FXMLElement* ConfigsElement);

	void DeserializeConfigs(FBinaryReader& Reader, std::unordered_map<std::string, WAttributesMap>& ConfigNode);

	void SerializeConfig(FBinaryWriter& Writer, FXMLElement* ConfigElement);

	void SerializeComponents(FBinaryWriter& Writer, FXMLElement* ComponentsElement);

	void DeserializeComponents(FBinaryReader& Reader, TArray<TSharedPtr<FBlueprintComponentNode>>& AttachedComponents);

	void SerializeComponent(FBinaryWriter& Writer, FXMLElement* ComponentElement);

	void DeserializeComponent(FBinaryReader& Reader, TSharedPtr<FBlueprintComponentNode>& ComponentNode);

	void SerializeEvents(FBinaryWriter& Writer, FXMLElement* EventsElement);

	void DeserializeEvents(FBinaryReader& Reader);

	void SerializeEvent(FBinaryWriter& Writer, FXMLElement* EventElement);

	void DeserializeEvent(FBinaryReader& Reader);
};