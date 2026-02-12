#include "BlueprintAsset.h"
#include "AssetLoader.h"
#include "Utility/Serialization.h"
#include "Utility/FileIO.h"
#include "Utility/String.h"
#include "Actor/Actor.h"
#include "Component/ActorComponent.h"
#include <filesystem>

int CountChildElement(FXMLElement* Parent, const std::string& Name)
{
    int Count = 0;
    for (FXMLElement* Child = Parent->FirstChildElement(Name.c_str()); Child; Child = Child->NextSiblingElement(Name.c_str()))
    {
        ++Count;
    }

    return Count;
}

int CountAttribute(FXMLElement* Element)
{
    if (Element == nullptr)
    {
        return 0;
    }

    int Count = 0;
    for (const FXMLAttribute* Attribute = Element->FirstAttribute(); Attribute; Attribute = Attribute->Next())
    {
        ++Count;
    }

    return Count;
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
    std::filesystem::path p(SourcePath);

    std::filesystem::path binFolder = p.parent_path() / "bin";
    std::filesystem::create_directories(binFolder);

    std::filesystem::path fileNameWithBin = p.filename().wstring() + L"bin";

    std::wstring BinaryPath = (binFolder / fileNameWithBin).wstring();

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

void FBlueprintAsset::RegisterToFactory()
{
    FActorFactory::RegisterActor(mName, [&]()
        {
            TSharedPtr<AActor> Actor = FActorFactory::CreateActor<AActor>(mParentClass);
            Actor->LoadBlueprint(this);
            return Actor;
        }
    );
}

bool FBlueprintAsset::CheckIfNeedCompile(const std::wstring& Src, const std::wstring& Bin)
{
    if (!std::filesystem::exists(Bin)) return true;
    return std::filesystem::last_write_time(Src) > std::filesystem::last_write_time(Bin);
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

    Serialize(Writer, RootElement);

    return true;
}

void FBlueprintAsset::Serialize(FBinaryWriter& Writer, FXMLElement* RootElement)
{
    // ParentClass 쓰기
    const std::string Parent = RootElement->Name();
    Writer << Parent;

    SerializeConfigs(Writer, RootElement->FirstChildElement("Configs"));

    SerializeComponents(Writer, RootElement->FirstChildElement("Components"));

    SerializeEvents(Writer , RootElement->FirstChildElement("Events"));
}

void FBlueprintAsset::Deserialize(FBinaryReader& Reader)
{
    Reader >> mParentClass;

    DeserializeConfigs(Reader, mConfigs);

    DeserializeComponents(Reader, mAttachedComponents);

    DeserializeEvents(Reader);
}

void FBlueprintAsset::SerializeAttributes(FBinaryWriter& Writer, FXMLElement* Element)
{
    int NumAttributes = CountAttribute(Element);
    Writer << NumAttributes;

    const FXMLAttribute* Attribute = Element->FirstAttribute();
    while (Attribute)
    {
        const std::string Name = Attribute->Name();
        const std::string Value = Attribute->Value();
        Writer << Name << Value;

        Attribute = Attribute->Next();
    }
}

void FBlueprintAsset::DeserializeAttributes(FBinaryReader& Reader, WAttributesMap& AttributesMap)
{
    int NumAttributes;
    Reader >> NumAttributes;

    for (int i = 0; i < NumAttributes; ++i)
    {
        std::string Name;
        std::string Value;
        Reader >> Name >> Value;
        AttributesMap[Name] = std::move(Value);
    }
}

void FBlueprintAsset::SerializeConfigs(FBinaryWriter& Writer, FXMLElement* ConfigsElement)
{
    if (ConfigsElement == nullptr)
    {
        Writer << (int)0;
        return;
    }

    int NumConfigs = ConfigsElement->ChildElementCount();
    Writer << NumConfigs;

    FXMLElement* ConfigElement = ConfigsElement->FirstChildElement();
    while (ConfigElement)
    {
        SerializeConfig(Writer, ConfigElement);
        ConfigElement = ConfigElement->NextSiblingElement();
    }
}

void FBlueprintAsset::DeserializeConfigs(FBinaryReader& Reader, std::unordered_map<std::string, WAttributesMap>& ConfigNode)
{
    int NumConfigs;
    Reader >> NumConfigs;

    for (int i = 0; i < NumConfigs; ++i)
    {
        std::string Name;
        Reader >> Name;
        DeserializeAttributes(Reader, ConfigNode[Name]);
    }
}

void FBlueprintAsset::SerializeConfig(FBinaryWriter& Writer, FXMLElement* ConfigElement)
{
    std::string Name = ConfigElement->Name();
    Writer << Name;

    SerializeAttributes(Writer, ConfigElement);
}

void FBlueprintAsset::SerializeComponents(FBinaryWriter& Writer, FXMLElement* ComponentsElement)
{
    if (ComponentsElement == nullptr)
    {
        Writer << int(0);
        return;
    }

    int NumComponents = ComponentsElement->ChildElementCount();
    Writer << NumComponents;

    FXMLElement* CompElement = ComponentsElement->FirstChildElement();
    while (CompElement)
    {
        SerializeComponent(Writer, CompElement);

        CompElement = CompElement->NextSiblingElement();
    }
}

void FBlueprintAsset::DeserializeComponents(FBinaryReader& Reader, TArray<TSharedPtr<FBlueprintComponentNode>>& AttachedComponents)
{
    int ComponentsNum;
    Reader >> ComponentsNum;

    AttachedComponents.resize(ComponentsNum);

    for (TSharedPtr<FBlueprintComponentNode>& Comp : AttachedComponents)
    {
        Comp = MakeShared<FBlueprintComponentNode>();
        DeserializeComponent(Reader, Comp);
    }
}

void FBlueprintAsset::SerializeComponent(FBinaryWriter& Writer, FXMLElement* ComponentElement)
{
    assert(ComponentElement && "ComponentElement is nullptr");

    const std::string Type = ComponentElement->Name();
    Writer << Type;

    SerializeAttributes(Writer, ComponentElement);

    SerializeComponents(Writer, ComponentElement);
}

void FBlueprintAsset::DeserializeComponent(FBinaryReader& Reader, TSharedPtr<FBlueprintComponentNode>& Component)
{
    Reader >> Component->Type;

    DeserializeAttributes(Reader, Component->Attributes);

    DeserializeComponents(Reader, Component->AttachedComponents);
}

void FBlueprintAsset::SerializeEvents(FBinaryWriter& Writer, FXMLElement* EventsElement)
{
    if (EventsElement == nullptr)
    {
        Writer << int(0);
        return;
    }

    int NumEvents = EventsElement->ChildElementCount();
    Writer << NumEvents;

    FXMLElement* EventElement = EventsElement->FirstChildElement();
    while (EventElement)
    {
        SerializeEvent(Writer, EventElement);
        EventElement = EventElement->NextSiblingElement();
    }
}

void FBlueprintAsset::DeserializeEvents(FBinaryReader& Reader)
{
    int NumEvents;
    Reader >> NumEvents;

    for (int i = 0; i < NumEvents; ++i)
    {
        DeserializeEvent(Reader);
    }
}

void FBlueprintAsset::SerializeEvent(FBinaryWriter& Writer, FXMLElement* EventElement)
{
    // 1. 이벤트 이름 (예: "OnSpawn", "OnHit")
    std::string EventName = EventElement->Name();
    Writer << EventName;

    SerializeAttributes(Writer, EventElement);

    // 2. 이 이벤트에 달린 액션 개수
    int NumActions = EventElement->ChildElementCount();
    Writer << NumActions;

    FXMLElement* ActionElement = EventElement->FirstChildElement();
    while (ActionElement)
    {
        // 3. 액션 정보 (Name과 Attributes)
        std::string ActionName = ActionElement->Name();
        Writer << ActionName;

        SerializeAttributes(Writer, ActionElement);

        ActionElement = ActionElement->NextSiblingElement();
    }
}

void FBlueprintAsset::DeserializeEvent(FBinaryReader& Reader)
{
    // 1. 이벤트 이름 읽기
    TSharedPtr<FBlueprintEventNode> EventNode = MakeShared<FBlueprintEventNode>();
    Reader >> EventNode->Name;

    DeserializeAttributes(Reader, EventNode->Attributes);

    // 2. 액션 개수 읽기
    int NumActions;
    Reader >> NumActions;

    EventNode->Actions.resize(NumActions);
    for (int i = 0; i < NumActions; ++i)
    {
        EventNode->Actions[i] = MakeShared<FBlueprintActionNode>();

        // 3. 액션 이름 읽기
        Reader >> EventNode->Actions[i]->Name;

        // 4. 액션의 속성 맵 복구
        DeserializeAttributes(Reader, EventNode->Actions[i]->Attributes);
    }

    if (EventNode->Name.substr(0, 2) == "On")
    {
        mEvents.push_back(std::move(EventNode));
    }
    else
    {
        mCustomEvents.push_back(std::move(EventNode));
    }
}