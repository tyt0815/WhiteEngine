#include "BlueprintAsset.h"
#include "AssetLoader.h"
#include "Utility/Serialization.h"
#include "Utility/FileIO.h"
#include "Utility/String.h"
#include "Actor/Actor.h"
#include "WComponentRegistry.h"
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

    BindToActorFactory();

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

void FBlueprintAsset::BindToActorFactory()
{
    FActorFactory::RegisterActor(mName, [this]()
        {
            auto Actor = FActorFactory::GetInstance()->CreateActor<AActor>(mParentClass);

            if (Actor)
            {
                Actor->LoadBlueprint(this);
            }

            return Actor;
        });
}

void FBlueprintAsset::Serialize(FBinaryWriter& Writer, FXMLElement* RootElement)
{
    // ParentClass 쓰기
    const std::string Parent = RootElement->Name();
    Writer << Parent;

    SerializeComponents(Writer, RootElement->FirstChildElement("Components"));

    SerializeStateMachine(Writer, RootElement->FirstChildElement("StateMachine"));
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

void FBlueprintAsset::SerializeComponent(FBinaryWriter& Writer, FXMLElement* ComponentElement)
{
    assert(ComponentElement && "ComponentElement is nullptr");

    const std::string Type = ComponentElement->Name();
    Writer << Type;

    SerializeAttributes(Writer, ComponentElement);

    SerializeComponents(Writer, ComponentElement);
}

void FBlueprintAsset::SerializeStateMachine(FBinaryWriter& Writer, FXMLElement* StateMachineElement)
{
    if (StateMachineElement == nullptr)
    {
        Writer << false; // StateMachine 존재 여부
        return;
    }

    Writer << true;

    // 1. Initial State 이름 (예: "Spawn")
    std::string InitialState = "";
    if (const char* Initial = StateMachineElement->Attribute("Initial"))
    {
        InitialState = Initial;
    }
    Writer << InitialState;

    // 4. States 직렬화
    int NumStates = CountChildElement(StateMachineElement, "State");
    Writer << NumStates;

    FXMLElement* StateElement = StateMachineElement->FirstChildElement("State");
    while (StateElement)
    {
        SerializeState(Writer, StateElement);
        StateElement = StateElement->NextSiblingElement("State");
    }
}

void FBlueprintAsset::SerializeState(FBinaryWriter& Writer, FXMLElement* StateElement)
{
    // 1. State 기본 정보 (Name, Base 상속 여부)
    std::string StateName = "";
    if (const char* Name = StateElement->Attribute("Name")) StateName = Name;

    std::string BaseState = "";
    if (const char* Base = StateElement->Attribute("Base")) BaseState = Base;

    Writer << StateName << BaseState;

    // 2. State 내부의 이벤트들 (OnSpawn, OnUpdate, Custom1 등)
    // State 태그는 Attributes가 없을 수도 있지만, 확장성을 위해 처리
    SerializeAttributes(Writer, StateElement);

    int NumEventsInState = StateElement->ChildElementCount();
    Writer << NumEventsInState;

    FXMLElement* EventElement = StateElement->FirstChildElement();
    while (EventElement)
    {
        SerializeEvent(Writer, EventElement);
        EventElement = EventElement->NextSiblingElement();
    }
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

void FBlueprintAsset::Deserialize(FBinaryReader& Reader)
{
    Reader >> mParentClass;

    mComponentSetups.clear();
    DeserializeComponents(Reader, "");

    DeserializeStateMachine(Reader);
}

void FBlueprintAsset::DeserializeAttributes(FBinaryReader& Reader, WAttributesMap& OutAttr)
{
    int Count;
    Reader >> Count;
    for (int i = 0; i < Count; ++i)
    {
        std::string Key, Value;
        Reader >> Key >> Value;
        OutAttr[Key] = std::move(Value);
    }
}

void FBlueprintAsset::DeserializeComponents(FBinaryReader& Reader, std::string ParentName)
{
    int NumComponents;
    Reader >> NumComponents;

    for (int i = 0; i < NumComponents; ++i)
    {
        std::string Type;
        Reader >> Type;

        WAttributesMap Attributes;
        DeserializeAttributes(Reader, Attributes);

        std::string MyName = Attributes["Name"];

        // [Registry 활용] 컴포넌트 생성 공장 정의
        auto CreateFunc = [Type, Attributes](AActor* Owner) -> WActorComponent* {
            return WComponentRegistry::Create(Type, Owner, Attributes);
        };

        mComponentSetups.push_back({ MyName, ParentName, Type, Attributes, CreateFunc });

        DeserializeComponents(Reader, MyName);
    }
}

void FBlueprintAsset::DeserializeStateMachine(FBinaryReader& Reader)
{
    Reader >> mRuntimeStateMachine.bExist;
    if (!mRuntimeStateMachine.bExist) return;

    Reader >> mRuntimeStateMachine.InitialState;

    // States 복구
    int NumStates;
    Reader >> NumStates;
    for (int i = 0; i < NumStates; ++i)
    {
        FStateRuntimeSetup StateSetup;
        Reader >> StateSetup.Name;
        Reader >> StateSetup.BaseName;

        // State 자체의 속성 (필요 시)
        WAttributesMap StateAttr;
        DeserializeAttributes(Reader, StateAttr);

        // State 내부 이벤트들
        DeserializeEvents(Reader, StateSetup.EventBindings);

        mRuntimeStateMachine.States[StateSetup.Name] = std::move(StateSetup);
    }
}

void FBlueprintAsset::DeserializeEvents(FBinaryReader& Reader, std::vector<FEventRuntimeBinding>& OutBindings)
{
    int NumEvents;
    Reader >> NumEvents;
    OutBindings.reserve(OutBindings.size() + NumEvents);

    for (int i = 0; i < NumEvents; ++i)
    {
        FEventRuntimeBinding Binding;
        Reader >> Binding.Tag; // OnSpawn, OnHit 등
        DeserializeAttributes(Reader, Binding.Attributes);

        int NumActions;
        Reader >> NumActions;
        for (int j = 0; j < NumActions; ++j)
        {
            std::string ActionTag;
            Reader >> ActionTag;
            WAttributesMap ActionAttr;
            DeserializeAttributes(Reader, ActionAttr);

            WActionFactory Factory = [ActionTag, ActionAttr](WObject* Target) -> WActionLambda {
                return WActionRegistry::Create(ActionTag, Target, ActionAttr);
            };

            Binding.ActionFactories.push_back(std::move(Factory));
        }
        OutBindings.push_back(std::move(Binding));
    }
}