#include "BlueprintAsset.h"
#include "AssetLoader.h"
#include "Utility/Serialization.h"
#include "Utility/FileIO.h"

using namespace BlueprintAsset;

int CountChildElement(FXMLElement* Parent, const std::string& Name)
{
    int Count = 0;
    for (FXMLElement* Child = Parent->FirstChildElement(Name.c_str()); Child; Child = Child->NextSiblingElement(Name.c_str()))
    {
        ++Count;
    }

    return Count;
}

bool FBlueprintAsset::LoadAsset(const std::wstring& FilePath)
{
    FBlueprintAssetCompiler Compiler;

    TArray<unsigned char> RawBuffer;

    if (!Compiler.SmartLoad(FilePath, RawBuffer))
    {
        return false;
    }

    // 1. 액터 기본 정보 읽기
    FBinaryReader Reader(RawBuffer);
    Reader >> mActorNode.ParentClass;

    // 2. 프로퍼티 읽기
    DeserializeProperties(mActorNode.Properties, Reader);

    // 3. 컴포넌트 읽기
    DeserializeComponents(Reader);

    return true;
}

void ReadProperties(EPropertyType Type, FBinaryReader& Reader, TArray<FProperty>& Properties)
{
    int PropertiesNum;
    Reader >> PropertiesNum;
    for (int i = 0; i < PropertiesNum; ++i)
    {
        FProperty Prop;
        Prop.Type = Type;
        Reader >> Prop.Name; // 이름 복구

        if (Type == EPropertyType::EPT_Float) 
        {
            float v;
            Reader >> v;
            Prop.Value = v;
        }
        else if (Type == EPropertyType::EPT_Boolean) 
        {
            bool v;
            Reader >> v;
            Prop.Value = v;
        }
        else 
        {
            std::string v;
            Reader >> v;
            Prop.Value = std::move(v);
        }
        Properties.push_back(std::move(Prop));
    }
}

void FBlueprintAsset::DeserializeProperties(TArray<FProperty>& Properties, FBinaryReader& Reader)
{
    // 1. 총 프로퍼티의 수 읽기
    int PropertyNum;
    Reader >> PropertyNum;

    // 2. 각각의 프로퍼티 읽기
    for (int i = 0; i < (int)EPropertyType::EPT_TypeNum; ++i)
    {
        ReadProperties((EPropertyType)i, Reader, Properties);
    }
}

void FBlueprintAsset::DeserializeInitializers(TArray<BlueprintAsset::FInitializer>& Initializers, FBinaryReader& Reader)
{
    int InitializersNum;
    Reader >> InitializersNum;

    for (int i = 0; i < InitializersNum; ++i)
    {
        FInitializer Init;
        Reader >> Init.Name;
        Reader >> Init.Value;
        Initializers.push_back(std::move(Init));
    }
}

void FBlueprintAsset::DeserializeComponents(FBinaryReader& Reader)
{
    // 1. 총 컴포넌트의 수 읽기
    int ComponentsNum;
    Reader >> ComponentsNum;

    mActorNode.Components.resize(ComponentsNum);
    for (int i = 0; i < ComponentsNum; ++i)
    {
        mActorNode.Components[i] = MakeShared<FComponentNode>();
        DeserializeComponent(mActorNode.Components[i].get(), Reader);
    }
}

void FBlueprintAsset::DeserializeComponent(FComponentNode* CompNode, FBinaryReader& Reader)
{
    // 2. 컴포넌트 기본 정보 읽기
    Reader >> CompNode->Class;
    Reader >> CompNode->Name;

    // 3. 컴포넌트 프로퍼티 쓰기
    DeserializeProperties(CompNode->Properties, Reader);

    DeserializeInitializers(CompNode->Initializers, Reader);
}

bool FBlueprintAssetCompiler::OnCompile(const std::wstring& SrcPath, std::vector<unsigned char>& OutBuffer)
{
    tinyxml2::XMLDocument Document;
    if (!Asset::LoadXML(SrcPath, Document))
    {
        return false;
    }

    tinyxml2::XMLElement* RootElement = Document.FirstChildElement();
    if (!RootElement)
    {
        return false;
    }

    std::string ParentClass = RootElement->Attribute("Parent");

    FBinaryWriter Writer(OutBuffer);

    // 1. 액터 기본정보 쓰기
    Writer << ParentClass;

    // 2. 액터 프로퍼티 쓰기
    SerializeProperties(RootElement->FirstChildElement("Properties"), Writer);

    // 3. 컴포넌트 쓰기
    SerializeComponents(RootElement->FirstChildElement("Components"), Writer);

    return true;
}

template<typename T>
void WriteProperty(const TArray<std::pair<std::string, T>>& Properties, FBinaryWriter& Writer)
{
    int PropertiesNum = (int)Properties.size();
    Writer << PropertiesNum;
    for (const auto& Pair : Properties)
    {
        Writer << Pair.first;
        Writer << Pair.second;
    }
}

void FBlueprintAssetCompiler::SerializeProperties(FXMLElement* PropertiesElement, FBinaryWriter& Writer)
{
    if (PropertiesElement == nullptr)
    {
        Writer << (int)0;
        return;
    }

    // 1. 총 프로퍼티의 수 쓰기
    int PropertiesNum = PropertiesElement->ChildElementCount();
    Writer << PropertiesNum;

    TArray<std::pair<std::string, float>> FloatProperties;
    TArray<std::pair<std::string, bool>> BooleanProperties;
    TArray<std::pair<std::string, std::string>> RawProperties;

    FXMLElement* PropertyElement = PropertiesElement->FirstChildElement();
    while (PropertyElement)
    {
        const std::string PropertyType = PropertyElement->Name();
        if (PropertyType == "float")
        {
            FloatProperties.push_back({ PropertyElement->Attribute("Name"), PropertyElement->FloatAttribute("Value") });
        }
        else if (PropertyType == "bool")
        {
            BooleanProperties.push_back({ PropertyElement->Attribute("Name"), PropertyElement->BoolAttribute("Value") });
        }
        else
        {
            RawProperties.push_back({ PropertyElement->Attribute("Name"), PropertyElement->Attribute("Value") });
        }

        PropertyElement = PropertyElement->NextSiblingElement();
    }

    // 2. 각각의 프로퍼티 쓰기
    // EPropertyType의 순서에 맞게 호출해야함
    WriteProperty(FloatProperties, Writer);
    WriteProperty(BooleanProperties, Writer);
    WriteProperty(RawProperties, Writer);

    assert(PropertiesNum == FloatProperties.size() + BooleanProperties.size() + RawProperties.size());
}

void FBlueprintAssetCompiler::SerializeInitializer(FXMLElement* InitializersElement, FBinaryWriter& Writer)
{
    if (!InitializersElement)
    {
        Writer << (int)0;
        return;
    }

    int InitializersNum = InitializersElement->ChildElementCount();
    Writer << InitializersNum;

    FXMLElement* InitElement = InitializersElement->FirstChildElement();
    while (InitElement)
    {
        Writer << InitElement->Name();
        Writer << InitElement->GetText();

        InitElement = InitElement->NextSiblingElement();
    }
}

void FBlueprintAssetCompiler::SerializeComponents(FXMLElement* ComponentsElement, FBinaryWriter& Writer)
{
    if (ComponentsElement == nullptr)
    {
        Writer << (int)0;
        return;
    }

    // 1. 총 컴포넌트의 수 쓰기
    int ComponentsNum = ComponentsElement->ChildElementCount();
    Writer << ComponentsNum;

    FXMLElement* CompElement = ComponentsElement->FirstChildElement();
    while (CompElement)
    {
        // 2. 컴포넌트 기본 정보 쓰기
        const std::string ComponentClass = CompElement->Name();
        Writer << ComponentClass;
        Writer << CompElement->Attribute("Name");

        FXMLElement* ChildElement = CompElement->FirstChildElement();

        // 3. 컴포넌트 프로퍼티 쓰기
        SerializeProperties(CompElement->FirstChildElement("Properties"), Writer);

        SerializeInitializer(CompElement->FirstChildElement("Initializers"), Writer);

        CompElement = CompElement->NextSiblingElement();
    }
}
