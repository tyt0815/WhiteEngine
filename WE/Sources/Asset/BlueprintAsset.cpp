#include "BlueprintAsset.h"
#include "AssetLoader.h"
#include "Utility/Serialization.h"
#include "Utility/FileIO.h"
#include "Utility/String.h"
#include "Actor/Actor.h"
#include "Component/ActorComponent.h"
#include <filesystem>

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

bool FBlueprintAsset::OnCompile(const std::wstring& SrcPath, std::vector<unsigned char>& OutBuffer)
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

    FBinaryWriter Writer(OutBuffer);

    Serialize(RootElement, Writer);

    return true;
}

void FBlueprintAsset::SerializeProperties(FXMLElement* PropertiesElement, FBinaryWriter& Writer)
{
    if (PropertiesElement == nullptr)
    {
        Writer << (int)0;
        return;
    }

    // 1. 총 프로퍼티의 수 쓰기
    int PropertiesNum = PropertiesElement->ChildElementCount();
    Writer << PropertiesNum;

    if (PropertiesNum == 0)
    {
        return;
    }

    TArray<std::pair<std::string, float>> FloatProperties;
    TArray<std::pair<std::string, bool>> BooleanProperties;
    TArray<std::pair<std::string, std::string>> StringProperties;

    FXMLElement* PropertyElement = PropertiesElement->FirstChildElement();
    while (PropertyElement)
    {
        SerializeProperty(PropertyElement, Writer);

        PropertyElement = PropertyElement->NextSiblingElement();
    }
}

void FBlueprintAsset::SerializeProperty(FXMLElement* PropertyElement, FBinaryWriter& Writer)
{
    const std::string PropertyType = PropertyElement->Name();

    Writer << PropertyElement->Attribute("Name");
    
    if (PropertyType == "float")
    {
        Writer << (int)EPropertyType::EPT_Float;
        const float Value = PropertyElement->FloatAttribute("Value");
        Writer << Value;
    }
    else if (PropertyType == "bool")
    {
        Writer << (int)EPropertyType::EPT_Boolean;
        const bool Value = PropertyElement->BoolAttribute("Value");;
        Writer << Value;
    }
    else if (PropertyType == "string")
    {
        Writer << (int)EPropertyType::EPT_String;
        const std::string Value = PropertyElement->Attribute("Value");
        Writer << Value;
    }
    else
    {
        assert(false && "Undefined Property");
    }
}

void FBlueprintAsset::DeserializeComponent(FBinaryReader& Reader, BlueprintAsset::FComponentNode* CompNode)
{
    Reader >> CompNode->ParentClass;

    DeserializeProperties(CompNode->Properties, Reader);

    DeserializeEvents(Reader, CompNode->Events);
}

void FBlueprintAsset::SerializeEvents(FXMLElement* EventsElement, FBinaryWriter& Writer)
{
    if (EventsElement == nullptr)
    {
        Writer << (int)0;
        return;
    }

    int EventsNum = EventsElement->ChildElementCount();
    Writer << EventsNum;

    FXMLElement* EventElement = EventsElement->FirstChildElement();
    while (EventElement)
    {
        SerializeEvent(EventElement, Writer);

        EventElement = EventElement->NextSiblingElement();
    }
}

void FBlueprintAsset::SerializeEvent(FXMLElement* EventElement, FBinaryWriter& Writer)
{
    if (EventElement == nullptr)
    {
        Writer << (int)0;
        return;
    }

    std::string EventName = EventElement->Name();
    int FunctionNum = EventElement->ChildElementCount();
    Writer << FunctionNum;
    if (FunctionNum == 0)
    {
        return;
    }
    Writer << EventName;

    FXMLElement* FunctionElement = EventElement->FirstChildElement();
    while (FunctionElement)
    {
        SerializeFunction(FunctionElement, Writer);

        FunctionElement = FunctionElement->NextSiblingElement();
    }
}

void FBlueprintAsset::SerializeFunction(FXMLElement* FuncElement, FBinaryWriter& Writer)
{
    const std::string FunctionName = FuncElement->Attribute("Func");
    std::string Target = FuncElement->Attribute("Target");
    std::string Name;
    if (const char* Attribute = FuncElement->Attribute("Name"))
    {
        Name = Attribute;
    }
    Writer << FunctionName << Target << Name;

    int ParametersNum = FuncElement->ChildElementCount();
    Writer << ParametersNum;

    FXMLElement* ParamElement = FuncElement->FirstChildElement();
    while (ParamElement)
    {
        std::string ElementName = ParamElement->Name();
        // 함수
        if (ElementName == "Call")
        {
            Writer << (int)EFunctionParameterType::EFPT_Function;
            SerializeFunction(ParamElement, Writer);
        }
        // 정적인 값
        else
        {
            Writer << (int)EFunctionParameterType::EFPT_Property;
            SerializeProperty(ParamElement, Writer);
        }

        ParamElement = ParamElement->NextSiblingElement();
    }
}

void FBlueprintAsset::DeserializeEvents(FBinaryReader& Reader, TArray<BlueprintAsset::FEventNode>& EventsNode)
{
    int EventsNum;
    Reader >> EventsNum;
    if (EventsNum == 0)
    {
        return;
    }

    EventsNode.resize(EventsNum);
    for (int i = 0; i < EventsNum; ++i)
    {
        DeserializeEvent(Reader, &EventsNode[i]);
    }
}

void FBlueprintAsset::DeserializeEvent(FBinaryReader& Reader, BlueprintAsset::FEventNode* EventNode)
{
    int FunctionNum;
    Reader >> FunctionNum;

    if (FunctionNum == 0)
    {
        return;
    }
    Reader >> EventNode->Name;

    EventNode->Functions.resize(FunctionNum);
    for (auto& Func : EventNode->Functions)
    {
        Func = MakeShared<FFunctionNode>();
        DeserializeFunction(Reader, Func.get());
    }
}

void FBlueprintAsset::DeserializeFunction(FBinaryReader& Reader, BlueprintAsset::FFunctionNode* FuncNode)
{
    Reader >> FuncNode->Call >> FuncNode->Target >> FuncNode->Name;

    int ParametersNum;
    Reader >> ParametersNum;

    for (int i = 0; i < ParametersNum; ++i)
    {
        int ParameterType;
        Reader >> ParameterType;

        if (ParameterType == (int)EFunctionParameterType::EFPT_Function)
        {
            FuncNode->FunctionParameters.push_back(MakeShared<FFunctionNode>());
            DeserializeFunction(Reader, FuncNode->FunctionParameters.back().get());
        }
        else
        {
            FuncNode->StaticParameters.push_back(FProperty());
            DeserializeProperty(FuncNode->StaticParameters.back(), Reader);
        }
    }
}

bool FBlueprintAsset::LoadAsset(const std::wstring& FilePath)
{
    TArray<unsigned char> RawBuffer;

    if (!SmartLoad(FilePath, RawBuffer))
    {
        return false;
    }

    // 1. 액터 기본 정보 읽기
    FBinaryReader Reader(RawBuffer);
    
    Deserialize(Reader);

    RegisterToFactory();

    return true;
}

bool FBlueprintAsset::SmartLoad(const std::wstring& SourcePath, TArray<unsigned char>& RawBuffer)
{
    std::wstring BinaryPath = SourcePath + L"bin";

    // 살짝 보강한다면
    if (CheckIfNeedCompile(SourcePath, BinaryPath))
    {
        std::vector<unsigned char> Buffer;
        if (!OnCompile(SourcePath, Buffer))
        {
            return false;
        }
        FileIO::SaveBufferToFile(BinaryPath, Buffer);
    }

    return FileIO::LoadBufferFromFile(BinaryPath, RawBuffer);
}

bool FBlueprintAsset::CheckIfNeedCompile(const std::wstring& Src, const std::wstring& Bin)
{
    if (!std::filesystem::exists(Bin)) return true;
    return std::filesystem::last_write_time(Src) > std::filesystem::last_write_time(Bin);
}

void FBlueprintAsset::DeserializeProperties(TArray<FProperty>& Properties, FBinaryReader& Reader)
{
    // 1. 총 프로퍼티의 수 읽기
    int PropertyNum;
    Reader >> PropertyNum;

    if (PropertyNum == 0)
    {
        return;
    }

    Properties.resize(PropertyNum);
    // 2. 각각의 프로퍼티 읽기
    for (int i = 0; i < PropertyNum; ++i)
    {
        DeserializeProperty(Properties[i], Reader);
    }
}

void FBlueprintAsset::DeserializeProperty(BlueprintAsset::FProperty& Property, FBinaryReader& Reader)
{
    Reader >> Property.Name;
    int Type;
    Reader >> Type;
    Property.Type = (EPropertyType)Type;
    if (Type == (int)EPropertyType::EPT_Float)
    {
        float Value;
        Reader >> Value;
        Property.Value = Value;
    }
    else if (Type == (int)EPropertyType::EPT_Boolean)
    {
        bool Value;
        Reader >> Value;
        Property.Value = Value;
    }
    else if (Type == (int)EPropertyType::EPT_String)
    {
        std::string Value;
        Reader >> Value;
        Property.Value = Value;
    }
    else
    {
        assert(false && "Undefined Property");
    }
}

void FBlueprintAsset::SerializeComponent(FXMLElement* ComponentElement, FBinaryWriter& Writer)
{
    const std::string ParentClass = ComponentElement->Name();
    Writer << ParentClass;

    
    SerializeProperties(ComponentElement->FirstChildElement("Properties"), Writer);

    SerializeEvents(ComponentElement->FirstChildElement("Events"), Writer);
}

void FActorBlueprintAsset::Serialize(FXMLElement* RootElement, FBinaryWriter& Writer)
{
    // 1. 액터 기본정보 쓰기
    std::string ParentClass = RootElement->Name();
    Writer << ParentClass;

    // 2. 액터 프로퍼티 쓰기
    SerializeProperties(RootElement->FirstChildElement("Properties"), Writer);

    // 3. 컴포넌트 쓰기
    SerializeAttachedComponents(RootElement->FirstChildElement("Components"), Writer);

    SerializeEvents(RootElement->FirstChildElement("Events"), Writer);
}

void FActorBlueprintAsset::Deserialize(FBinaryReader& Reader)
{
    Reader >> mRootNode.ParentClass;

    // 2. 프로퍼티 읽기
    DeserializeProperties(mRootNode.Properties, Reader);

    // 3. 컴포넌트 읽기
    DeserializeAttachedComponents(Reader);

    DeserializeEvents(Reader, mRootNode.Events);
}

void FActorBlueprintAsset::SerializeAttachedComponents(FXMLElement* AttachedComponentsElement, FBinaryWriter& Writer)
{
    if (AttachedComponentsElement == nullptr)
    {
        Writer << (int)0;
        return;
    }

    // 1. 총 컴포넌트의 수 쓰기
    int ComponentsNum = AttachedComponentsElement->ChildElementCount();
    Writer << ComponentsNum;

    FXMLElement* CompElement = AttachedComponentsElement->FirstChildElement();
    while (CompElement)
    {
        // 2. 컴포넌트 기본 정보 쓰기
        const std::string ComponentName = CompElement->Attribute("Name");
        Writer << ComponentName;

        SerializeComponent(CompElement, Writer);

        CompElement = CompElement->NextSiblingElement();
    }
}

void FActorBlueprintAsset::DeserializeAttachedComponents(FBinaryReader& Reader)
{
    // 1. 총 컴포넌트의 수 읽기
    int ComponentsNum;
    Reader >> ComponentsNum;

    TArray<TSharedPtr<BlueprintAsset::FAttachedComponentNode>>& Components = mRootNode.AttachedComponents;
    Components.clear();
    Components.resize(ComponentsNum);
    for (int i = 0; i < ComponentsNum; ++i)
    {
        Components[i] = MakeShared<FAttachedComponentNode>();
        DeserializeAttachedComponent(Components[i].get(), Reader);
    }
}

void FActorBlueprintAsset::DeserializeAttachedComponent(BlueprintAsset::FAttachedComponentNode* AttachedCompNode, FBinaryReader& Reader)
{
    // 2. 컴포넌트 기본 정보 읽기
    Reader >> AttachedCompNode->Name;

    DeserializeComponent(Reader, &AttachedCompNode->ComponentNode);
}

void FActorBlueprintAsset::RegisterToFactory()
{
    FActorFactory::RegisterActor(WStringToString(mName), [&]()
        {
            TSharedPtr<AActor> Actor = FActorFactory::CreateActor<AActor>(mRootNode.ParentClass);
            Actor->LoadBlueprint(&mRootNode);
            return Actor;
        }
    );
}

void FComponentBlueprintAsset::Serialize(FXMLElement* RootElement, FBinaryWriter& Writer)
{
    SerializeComponent(RootElement, Writer);
}

void FComponentBlueprintAsset::Deserialize(FBinaryReader& Reader)
{
    DeserializeComponent(Reader, &mRootNode);
}

void FComponentBlueprintAsset::RegisterToFactory()
{
    FComponentFactory::RegisterComponent(WStringToString(mName), [&]()
        {
            TSharedPtr<WActorComponent> Comp = FComponentFactory::CreateComponent<WActorComponent>(mRootNode.ParentClass);
            Comp->LoadBlueprint(Comp.get(), &mRootNode);
            return Comp;
        }
    );
}
