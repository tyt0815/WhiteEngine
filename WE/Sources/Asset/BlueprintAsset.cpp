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
        const std::string PropertyType = PropertyElement->Name();
        if (PropertyType == "float")
        {
            FloatProperties.push_back({ PropertyElement->Attribute("Name"), PropertyElement->FloatAttribute("Value") });
        }
        else if (PropertyType == "bool")
        {
            BooleanProperties.push_back({ PropertyElement->Attribute("Name"), PropertyElement->BoolAttribute("Value") });
        }
        else if (PropertyType == "string")
        {
            StringProperties.push_back({ PropertyElement->Attribute("Name"), PropertyElement->Attribute("Value") });
        }
        else
        {
            assert(false && "Undefined Property");
        }

        PropertyElement = PropertyElement->NextSiblingElement();
    }

    // 2. 각각의 프로퍼티 쓰기
    // EPropertyType의 순서에 맞게 호출해야함
    WriteProperty(FloatProperties, Writer);
    WriteProperty(BooleanProperties, Writer);
    WriteProperty(StringProperties, Writer);
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
        std::string Target = FunctionElement->Attribute("Target");
        std::string Name = FunctionElement->Attribute("Name");
        Writer << Target << Name;

        const FXMLAttribute* Attribute = FunctionElement->FirstAttribute();
        TArray<FInputParameter> Params;
        while (Attribute)
        {
            const std::string AttributeName = Attribute->Name();
            if (AttributeName == "Target" || AttributeName == "Name")
            {
                Attribute = Attribute->Next();
                continue;
            }
            
            const std::string Value = Attribute->Value();
            Params.push_back({ AttributeName, Value });

            Attribute = Attribute->Next();
        }

        Writer << (int)Params.size();
        for (const auto& Param : Params)
        {
            Writer << Param.Name << Param.Value;
        }

        FunctionElement = FunctionElement->NextSiblingElement();
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
        Reader >> Func.Target >> Func.Name;

        int InputsNum;
        Reader >> InputsNum;
        Func.Inputs.resize(InputsNum);
        for (auto& Input : Func.Inputs)
        {
            Reader >> Input.Name >> Input.Value;
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
        else if (Type == EPropertyType::EPT_String)
        {
            std::string v;
            Reader >> v;
            Prop.Value = v;
        }
        else 
        {
            assert(false && "Undefined Property");
        }
        Properties.push_back(std::move(Prop));
    }
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

    // 2. 각각의 프로퍼티 읽기
    for (int i = 0; i < (int)EPropertyType::EPT_TypeNum; ++i)
    {
        ReadProperties((EPropertyType)i, Reader, Properties);
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
}

void FActorBlueprintAsset::Deserialize(FBinaryReader& Reader)
{
    Reader >> mRootNode.ParentClass;

    // 2. 프로퍼티 읽기
    DeserializeProperties(mRootNode.Properties, Reader);

    // 3. 컴포넌트 읽기
    DeserializeAttachedComponents(Reader);
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
            Comp->LoadBlueprint(&mRootNode);
            return Comp;
        }
    );
}
