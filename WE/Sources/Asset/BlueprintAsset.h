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

//namespace BlueprintAsset
//{
//	enum class EPropertyType : int
//	{
//		EPT_Boolean,
//		EPT_Int,
//		EPT_Float,
//		EPT_Float3,
//		EPT_String,
//		EPT_StringArray,
//		EPT_TypeNum
//	};
//
//	template<typename T> struct PropertyTraits;
//
//	template<> struct PropertyTraits<bool>
//	{
//		static constexpr const char* Tag = "bool";
//		static constexpr EPropertyType Type = EPropertyType::EPT_Boolean;
//
//		static bool Parse(FXMLElement* Element) { return Element->BoolAttribute("Value"); }
//	};
//
//	template<> struct PropertyTraits<int>
//	{
//		static constexpr const char* Tag = "int";
//		static constexpr EPropertyType Type = EPropertyType::EPT_Int;
//
//		static int Parse(FXMLElement* Element) { return Element->IntAttribute("Value"); }
//	};
//
//	template<> struct PropertyTraits<float>
//	{
//		static constexpr const char* Tag = "float";
//		static constexpr EPropertyType Type = EPropertyType::EPT_Float;
//
//		static float Parse(FXMLElement* Element) { return Element->FloatAttribute("Value"); }
//	};
//
//	template<> struct PropertyTraits<XMFLOAT3>
//	{
//		static constexpr const char* Tag = "float3";
//		static constexpr EPropertyType Type = EPropertyType::EPT_Float3;
//
//		static XMFLOAT3 Parse(FXMLElement* Element)
//		{
//			std::string Float3 = Element->Attribute("Value");
//			XMFLOAT3 Value;
//			sscanf_s(Float3.c_str(), "(%f, %f, %f)", &Value.x, &Value.y, &Value.z);
//
//			return Value;
//		}
//	};
//
//	template<> struct PropertyTraits<std::string>
//	{
//		static constexpr const char* Tag = "string";
//		static constexpr EPropertyType Type = EPropertyType::EPT_String;
//
//		static std::string Parse(FXMLElement* Element) { return Element->Attribute("Value"); }
//	};
//
//	template<> struct PropertyTraits<std::vector<std::string>>
//	{
//		static constexpr const char* Tag = "array_string";
//		static constexpr EPropertyType Type = EPropertyType::EPT_StringArray;
//
//		static std::vector<std::string> Parse(FXMLElement* Element)
//		{
//			std::vector<std::string> Values;
//			FXMLElement* ValueElement = Element->FirstChildElement();
//			while (ValueElement)
//			{
//				Values.push_back(ValueElement->Attribute("Value"));
//				ValueElement = ValueElement->NextSiblingElement();
//			}
//
//			return Values;
//		}
//	};
//
//	struct FProperty
//	{
//		using FPropertyValue = std::variant<
//			bool,
//			int,
//			float,
//			XMFLOAT3,
//			std::string,
//			std::vector<std::string>
//		>;
//
//		std::string Name;
//		EPropertyType Type;
//		FPropertyValue Value;
//	};
//
//	enum class EFunctionParameterType : int
//	{
//		EFPT_Function = 0,
//		EFPT_Property,
//		EFPT_ConstValue
//	};
//
//	struct FPropertyParameter
//	{
//		std::string Name;
//		std::string Target;
//		std::string Value;
//		EPropertyType Type;
//	};
//
//	struct FFunctionNode
//	{
//		// 호출되는 함수 이름
//		std::string Call;
//
//		// 함수를 호출할 객체
//		std::string Target;
//
//		// 함수의 리턴이 반환될 WProperty 이름
//		std::string Name;
//
//		TArray<FProperty> StaticParameters;
//
//		TArray<FPropertyParameter> PropertyParameters;
//
//		TArray<TSharedPtr<FFunctionNode>> FunctionParameters;
//	};
//
//	struct FEventNode
//	{
//		std::string Name;
//		TArray<TSharedPtr<FFunctionNode>> Functions;
//	};
//
//	struct FComponentNode
//	{
//		std::string ParentClass;
//
//		TArray<FProperty> Properties;
//
//		TArray<FProperty> Variables;
//
//		TArray<FEventNode> Events;
//	};
//
//	struct FAttachedComponentNode
//	{
//		std::string Name;
//
//		FComponentNode ComponentNode;
//	};
//
//	struct FActorNode
//	{
//		std::string ParentClass;
//
//		TArray<FProperty> Properties; 
//
//		TArray<FProperty> Variables;
//
//		TArray<TSharedPtr<FAttachedComponentNode>> AttachedComponents;
//
//		TArray<FEventNode> Events;
//	};
//}

using FBlueprintAttributesMap = std::unordered_map<std::string, std::string>;

struct FComponentNode
{
	std::string Type;
	FBlueprintAttributesMap Attributes;

	TArray<TSharedPtr<FComponentNode>> AttachedComponents;
};

class FBlueprintAsset : public FAsset
{
	typedef FAsset Super;

public:

	std::string mParentClass;

	FBlueprintAttributesMap mAttributes;

	TArray<TSharedPtr<FComponentNode>> mAttachedComponents;

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

	void DeserializeAttributes(FBinaryReader& Reader, FBlueprintAttributesMap& AttributesMap);

	void SerializeComponents(FBinaryWriter& Writer, FXMLElement* ComponentsElement);

	void DeserializeComponents(FBinaryReader& Reader, TArray<TSharedPtr<FComponentNode>>& AttachedComponents);

	void SerializeComponent(FBinaryWriter& Writer, FXMLElement* ComponentElement);

	void DeserializeComponent(FBinaryReader& Reader, TSharedPtr<FComponentNode>& Component);
};