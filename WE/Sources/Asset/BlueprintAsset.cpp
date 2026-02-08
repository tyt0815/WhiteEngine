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

    SerializeAttributes(Writer, RootElement);

    SerializeComponents(Writer, RootElement->FirstChildElement("Components"));

    SerializeEvents(Writer , RootElement->FirstChildElement("Events"));
}

void FBlueprintAsset::Deserialize(FBinaryReader& Reader)
{
    Reader >> mParentClass;

    DeserializeAttributes(Reader, mAttributes);

    DeserializeComponents(Reader, mAttachedComponents);

    DeserializeEvents(Reader, mEvents);
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

void FBlueprintAsset::DeserializeEvents(FBinaryReader& Reader, TArray<TSharedPtr<FBlueprintEventNode>>& Events)
{
    int NumEvents;
    Reader >> NumEvents;

    Events.resize(NumEvents);
    for (int i = 0; i < NumEvents; ++i)
    {
        Events[i] = MakeShared<FBlueprintEventNode>();
        DeserializeEvent(Reader, Events[i]);
    }
}

void FBlueprintAsset::SerializeEvent(FBinaryWriter& Writer, FXMLElement* EventElement)
{
    // 1. 이벤트 이름 (예: "OnSpawn", "OnHit")
    std::string EventName = EventElement->Name();
    Writer << EventName;

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

void FBlueprintAsset::DeserializeEvent(FBinaryReader& Reader, TSharedPtr<FBlueprintEventNode>& EventNode)
{
    // 1. 이벤트 이름 읽기
    Reader >> EventNode->Name;

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
}

//void FBlueprintAsset::SerializeProperties(FXMLElement* PropertiesElement, FBinaryWriter& Writer)
//{
//    if (PropertiesElement == nullptr)
//    {
//        Writer << (int)0;
//        return;
//    }
//
//    // 1. 총 프로퍼티의 수 쓰기
//    int PropertiesNum = PropertiesElement->ChildElementCount();
//    Writer << PropertiesNum;
//
//    if (PropertiesNum == 0)
//    {
//        return;
//    }
//
//    TArray<std::pair<std::string, float>> FloatProperties;
//    TArray<std::pair<std::string, bool>> BooleanProperties;
//    TArray<std::pair<std::string, std::string>> StringProperties;
//
//    FXMLElement* PropertyElement = PropertiesElement->FirstChildElement();
//    while (PropertyElement)
//    {
//        SerializeProperty(PropertyElement, Writer);
//
//        PropertyElement = PropertyElement->NextSiblingElement();
//    }
//}
//
//template<typename... Args>
//EPropertyType GetPropertyTypeFromTag(const std::string& Tag, std::variant<Args...>)
//{
//    EPropertyType result = EPropertyType::EPT_TypeNum;
//    ((Tag == PropertyTraits<Args>::Tag ? (result = PropertyTraits<Args>::Type, true) : false) || ...);
//    return result;
//}
//
//template<typename... Args>
//void SerializePropertyHelper(FXMLElement* PropertyElement, FBinaryWriter& Writer, std::variant<Args...>)
//{
//    std::string Name = PropertyElement->Attribute("Name");
//    Writer << Name;
//
//    const std::string Tag = PropertyElement->Name();
//
//    // 폴드 표현식
//    bool bSuccess = ((Tag == PropertyTraits<Args>::Tag ? 
//        (Writer << (int)PropertyTraits<Args>::Type, Writer << PropertyTraits<Args>::Parse(PropertyElement), true) : false) ||
//    ...);
//
//
//    assert(bSuccess && "Invalid property type");
//}
//
//void FBlueprintAsset::SerializeProperty(FXMLElement* PropertyElement, FBinaryWriter& Writer)
//{
//    SerializePropertyHelper(PropertyElement, Writer, FProperty::FPropertyValue{});
//}
//
//void FBlueprintAsset::DeserializeComponent(FBinaryReader& Reader, BlueprintAsset::FComponentNode* CompNode)
//{
//    Reader >> CompNode->ParentClass;
//
//    DeserializeProperties(CompNode->Properties, Reader);
//
//    DeserializeProperties(CompNode->Variables, Reader);
//
//    DeserializeEvents(Reader, CompNode->Events);
//}
//
//void FBlueprintAsset::SerializeEvents(FXMLElement* EventsElement, FBinaryWriter& Writer)
//{
//    if (EventsElement == nullptr)
//    {
//        Writer << (int)0;
//        return;
//    }
//
//    int EventsNum = EventsElement->ChildElementCount();
//    Writer << EventsNum;
//
//    FXMLElement* EventElement = EventsElement->FirstChildElement();
//    while (EventElement)
//    {
//        SerializeEvent(EventElement, Writer);
//
//        EventElement = EventElement->NextSiblingElement();
//    }
//}
//
//void FBlueprintAsset::SerializeEvent(FXMLElement* EventElement, FBinaryWriter& Writer)
//{
//    if (EventElement == nullptr)
//    {
//        Writer << (int)0;
//        return;
//    }
//
//    std::string EventName = EventElement->Name();
//    int FunctionNum = EventElement->ChildElementCount();
//    Writer << FunctionNum;
//    if (FunctionNum == 0)
//    {
//        return;
//    }
//    Writer << EventName;
//
//    FXMLElement* FunctionElement = EventElement->FirstChildElement();
//    while (FunctionElement)
//    {
//        SerializeFunction(FunctionElement, Writer);
//
//        FunctionElement = FunctionElement->NextSiblingElement();
//    }
//}
//
//void FBlueprintAsset::SerializeFunction(FXMLElement* FuncElement, FBinaryWriter& Writer)
//{
//    const std::string FunctionName = FuncElement->Attribute("Func");
//    std::string Target = FuncElement->Attribute("Target");
//    std::string Name;
//    if (const char* Attribute = FuncElement->Attribute("Name"))
//    {
//        Name = Attribute;
//    }
//    Writer << FunctionName << Target << Name;
//
//    int ParametersNum = FuncElement->ChildElementCount();
//    Writer << ParametersNum;
//
//    FXMLElement* ParamElement = FuncElement->FirstChildElement();
//    while (ParamElement)
//    {
//        std::string ElementName = ParamElement->Name();
//        // 함수
//        if (ElementName == "Call")
//        {
//            Writer << (int)EFunctionParameterType::EFPT_Function;
//            SerializeFunction(ParamElement, Writer);
//        }
//        else
//        {
//            const char* Target = ParamElement->Attribute("Target");
//            if (Target)
//            {
//                Writer << (int)EFunctionParameterType::EFPT_Property;
//                int Type = (int)GetPropertyTypeFromTag(ParamElement->Name(), FProperty::FPropertyValue{});
//                std::string Name = ParamElement->Attribute("Name");
//                std::string TargetS = Target;
//                std::string Value = ParamElement->Attribute("Value");
//                Writer << Type << Name << TargetS << Value;
//            }
//            // 정적인 값
//            else
//            {
//                Writer << (int)EFunctionParameterType::EFPT_ConstValue;
//                SerializeProperty(ParamElement, Writer);
//            }
//        }
//
//        ParamElement = ParamElement->NextSiblingElement();
//    }
//}
//
//void FBlueprintAsset::DeserializeEvents(FBinaryReader& Reader, TArray<BlueprintAsset::FEventNode>& EventsNode)
//{
//    int EventsNum;
//    Reader >> EventsNum;
//    if (EventsNum == 0)
//    {
//        return;
//    }
//
//    EventsNode.resize(EventsNum);
//    for (int i = 0; i < EventsNum; ++i)
//    {
//        DeserializeEvent(Reader, &EventsNode[i]);
//    }
//}
//
//void FBlueprintAsset::DeserializeEvent(FBinaryReader& Reader, BlueprintAsset::FEventNode* EventNode)
//{
//    int FunctionNum;
//    Reader >> FunctionNum;
//
//    if (FunctionNum == 0)
//    {
//        return;
//    }
//    Reader >> EventNode->Name;
//
//    EventNode->Functions.resize(FunctionNum);
//    for (auto& Func : EventNode->Functions)
//    {
//        Func = MakeShared<FFunctionNode>();
//        DeserializeFunction(Reader, Func.get());
//    }
//}
//
//void FBlueprintAsset::DeserializeFunction(FBinaryReader& Reader, BlueprintAsset::FFunctionNode* FuncNode)
//{
//    Reader >> FuncNode->Call >> FuncNode->Target >> FuncNode->Name;
//
//    int ParametersNum;
//    Reader >> ParametersNum;
//
//    for (int i = 0; i < ParametersNum; ++i)
//    {
//        int ParameterType;
//        Reader >> ParameterType;
//
//        if (ParameterType == (int)EFunctionParameterType::EFPT_Function)
//        {
//            FuncNode->FunctionParameters.push_back(MakeShared<FFunctionNode>());
//            DeserializeFunction(Reader, FuncNode->FunctionParameters.back().get());
//        }
//        else if (ParameterType == (int)EFunctionParameterType::EFPT_Property)
//        {
//            FPropertyParameter Param;
//            int Type;
//            std::string Value;
//            Reader >> Type >> Param.Name >> Param.Target >> Value;
//            Param.Type = (EPropertyType)Type;
//            Param.Value = Value;
//            FuncNode->PropertyParameters.push_back(Param);
//        }
//        else
//        {
//            FuncNode->StaticParameters.push_back(FProperty());
//            DeserializeProperty(FuncNode->StaticParameters.back(), Reader);
//        }
//    }
//}

//void FBlueprintAsset::DeserializeProperties(TArray<FProperty>& Properties, FBinaryReader& Reader)
//{
//    // 1. 총 프로퍼티의 수 읽기
//    int PropertyNum;
//    Reader >> PropertyNum;
//
//    if (PropertyNum == 0)
//    {
//        return;
//    }
//
//    Properties.resize(PropertyNum);
//    // 2. 각각의 프로퍼티 읽기
//    for (int i = 0; i < PropertyNum; ++i)
//    {
//        DeserializeProperty(Properties[i], Reader);
//    }
//}
//
//template<typename... Args>
//void DeserializePropertyHelper(EPropertyType TargetType, FBinaryReader& Reader, std::variant<Args...>& OutValue)
//{
//    bool bSuccess = (
//        (PropertyTraits<Args>::Type == TargetType ?
//            ([&]() {
//                Args temp;
//                Reader >> temp;
//                OutValue = std::move(temp); // 성능을 위해 move 권장
//                return true;
//                }()) : false)
//        || ...);
//
//    assert(bSuccess && "Invalid Property");
//}
//
//void FBlueprintAsset::DeserializeProperty(BlueprintAsset::FProperty& Property, FBinaryReader& Reader)
//{
//    Reader >> Property.Name;
//    int Type;
//    Reader >> Type;
//    Property.Type = (EPropertyType)Type;
//    DeserializePropertyHelper(Property.Type, Reader, Property.Value);
//}
//
//void FBlueprintAsset::SerializeComponent(FXMLElement* ComponentElement, FBinaryWriter& Writer)
//{
//    const std::string ParentClass = ComponentElement->Name();
//    Writer << ParentClass;
//
//    
//    SerializeProperties(ComponentElement->FirstChildElement("Properties"), Writer);
//
//    SerializeProperties(ComponentElement->FirstChildElement("Variables"), Writer);
//
//    SerializeEvents(ComponentElement->FirstChildElement("Events"), Writer);
//}

//void FActorBlueprintAsset::Serialize(FXMLElement* RootElement, FBinaryWriter& Writer)
//{
//    // 1. 액터 기본정보 쓰기
//    std::string ParentClass = RootElement->Name();
//    Writer << ParentClass;
//
//    // 2. 액터 프로퍼티 쓰기
//    SerializeProperties(RootElement->FirstChildElement("Properties"), Writer);
//
//    SerializeProperties(RootElement->FirstChildElement("Variables"), Writer);
//
//    // 3. 컴포넌트 쓰기
//    SerializeAttachedComponents(RootElement->FirstChildElement("Components"), Writer);
//
//    SerializeEvents(RootElement->FirstChildElement("Events"), Writer);
//}
//
//void FActorBlueprintAsset::Deserialize(FBinaryReader& Reader)
//{
//    Reader >> mRootNode.ParentClass;
//
//    // 2. 프로퍼티 읽기
//    DeserializeProperties(mRootNode.Properties, Reader);
//
//    DeserializeProperties(mRootNode.Variables, Reader);
//
//    // 3. 컴포넌트 읽기
//    DeserializeAttachedComponents(Reader);
//
//    DeserializeEvents(Reader, mRootNode.Events);
//}
//
//void FActorBlueprintAsset::SerializeAttachedComponents(FXMLElement* AttachedComponentsElement, FBinaryWriter& Writer)
//{
//    if (AttachedComponentsElement == nullptr)
//    {
//        Writer << (int)0;
//        return;
//    }
//
//    // 1. 총 컴포넌트의 수 쓰기
//    int ComponentsNum = AttachedComponentsElement->ChildElementCount();
//    Writer << ComponentsNum;
//
//    FXMLElement* CompElement = AttachedComponentsElement->FirstChildElement();
//    while (CompElement)
//    {
//        // 2. 컴포넌트 기본 정보 쓰기
//        const char* NameCstr = CompElement->Attribute("Name");
//        assert(NameCstr && "Component name is not found");
//        const std::string ComponentName = NameCstr;
//        Writer << ComponentName;
//
//        SerializeComponent(CompElement, Writer);
//
//        CompElement = CompElement->NextSiblingElement();
//    }
//}
//
//void FActorBlueprintAsset::DeserializeAttachedComponents(FBinaryReader& Reader)
//{
//    // 1. 총 컴포넌트의 수 읽기
//    int ComponentsNum;
//    Reader >> ComponentsNum;
//
//    TArray<TSharedPtr<BlueprintAsset::FAttachedComponentNode>>& Components = mRootNode.AttachedComponents;
//    Components.clear();
//    Components.resize(ComponentsNum);
//    for (int i = 0; i < ComponentsNum; ++i)
//    {
//        Components[i] = MakeShared<FAttachedComponentNode>();
//        DeserializeAttachedComponent(Components[i].get(), Reader);
//    }
//}
//
//void FActorBlueprintAsset::DeserializeAttachedComponent(BlueprintAsset::FAttachedComponentNode* AttachedCompNode, FBinaryReader& Reader)
//{
//    // 2. 컴포넌트 기본 정보 읽기
//    Reader >> AttachedCompNode->Name;
//
//    DeserializeComponent(Reader, &AttachedCompNode->ComponentNode);
//}
//
//void FActorBlueprintAsset::RegisterToFactory()
//{
//    FActorFactory::RegisterActor(mName, [&]()
//        {
//            TSharedPtr<AActor> Actor = FActorFactory::CreateActor<AActor>(mRootNode.ParentClass);
//            Actor->LoadBlueprint(&mRootNode);
//            return Actor;
//        }
//    );
//}
//
//void FComponentBlueprintAsset::Serialize(FXMLElement* RootElement, FBinaryWriter& Writer)
//{
//    SerializeComponent(RootElement, Writer);
//}
//
//void FComponentBlueprintAsset::Deserialize(FBinaryReader& Reader)
//{
//    DeserializeComponent(Reader, &mRootNode);
//}
//
//void FComponentBlueprintAsset::RegisterToFactory()
//{
//    FComponentFactory::RegisterComponent(mName, [&]()
//        {
//            TSharedPtr<WActorComponent> Comp = FComponentFactory::CreateComponent<WActorComponent>(mRootNode.ParentClass);
//            Comp->LoadBlueprint(Comp.get(), &mRootNode);
//            return Comp;
//        }
//    );
//}
