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

    FBinaryReader Reader(RawBuffer);
    Reader >> mParentClass;

    // 프로퍼티
    DeserializeProperties(Reader);

    DeserializeComponents(Reader);

    return true;
}

void FBlueprintAsset::DeserializeProperties(FBinaryReader& Reader)
{
    int PropertiesNum;
    Reader >> PropertiesNum;
    if (PropertiesNum > 0)
    {
        int FloatAttributeNum;
        Reader >> FloatAttributeNum;
        for (int i = 0; i < FloatAttributeNum; ++i)
        {
            std::string Name;
            Reader >> Name;
            float Value;
            Reader >> Value;
            mFloatMap[Name] = Value;
        }
    }
}

void FBlueprintAsset::DeserializeComponents(FBinaryReader& Reader)
{
    // 컴포넌트
    int ComponentsNum;
    Reader >> ComponentsNum;
    if (ComponentsNum > 0)
    {
        int StaticMeshComponentsNum;
        Reader >> StaticMeshComponentsNum;

        for (int i = 0; i < StaticMeshComponentsNum; ++i)
        {
            std::string ComponentName;
            Reader >> ComponentName;
            FStaticMeshComponentInfo Info;
            Reader >> Info.StaticMesh;

            mStaticMeshComponentMap[ComponentName] = Info;
        }
    }
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

    Writer << ParentClass;

    SerializeProperties(RootElement->FirstChildElement("Properties"), Writer);

    SerializeComponents(RootElement->FirstChildElement("Components"), Writer);

    return true;
}

void FBlueprintAssetCompiler::SerializeProperties(FXMLElement* PropertiesElement, FBinaryWriter& Writer)
{
    if (PropertiesElement == nullptr)
    {
        Writer << (int)0;
        return;
    }

    int PropertiesNum = PropertiesElement->ChildElementCount();
    Writer << PropertiesNum;

    // Float 엘리먼트 컴파일
    int FloatElementCount = CountChildElement(PropertiesElement, "float");
    Writer << FloatElementCount;
    for (FXMLElement* FloatElement = PropertiesElement->FirstChildElement("float"); FloatElement; FloatElement = FloatElement->NextSiblingElement("float"))
    {
        Writer << FloatElement->Attribute("Name");
        Writer << FloatElement->FloatAttribute("Value");
    }

    assert(PropertiesNum == FloatElementCount);
}

void FBlueprintAssetCompiler::SerializeComponents(FXMLElement* ComponentsElement, FBinaryWriter& Writer)
{
    if (ComponentsElement == nullptr)
    {
        Writer << (int)0;
        return;
    }

    int ComponentsNum = ComponentsElement->ChildElementCount();
    Writer << ComponentsNum;

    // StaticMesh Component 엘리먼트 컴파일
    const char* StaticMeshComponentElementName = "StaticMeshComponent";
    int StaticMeshComponentCount = CountChildElement(ComponentsElement, StaticMeshComponentElementName);
    Writer << StaticMeshComponentCount;
    for (FXMLElement* StaticMeshComponentElement = ComponentsElement->FirstChildElement(StaticMeshComponentElementName); StaticMeshComponentElement; StaticMeshComponentElement = StaticMeshComponentElement->NextSiblingElement(StaticMeshComponentElementName))
    {
        // Child 내용
        std::string ComponentName = StaticMeshComponentElement->Attribute("Name");
        Writer << ComponentName;

        FStaticMeshComponentInfo Info;
        if (const FXMLElement* Element = StaticMeshComponentElement->FirstChildElement("StaticMesh"))
        {
            Info.StaticMesh = Element->GetText();
        }

        Writer << Info.StaticMesh;
    }
}
