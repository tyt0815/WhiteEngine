#pragma once

#include "Asset.h"
#include "Utility/Container.h"
#include "Utility/Memory.h"
#include "BlueprintTypes.h"
#include "WActionRegistry.h"
#include <tinyxml2.h>
#include <vector>

using FXMLDocument = tinyxml2::XMLDocument;
using FXMLElement = tinyxml2::XMLElement;
using FXMLAttribute = tinyxml2::XMLAttribute;

class FBinaryWriter;
class FBinaryReader;

class AActor;
class WActorComponent;

using WActionFactory = std::function<WActionLambda(WObject* Target)>;

struct FComponentRuntimeSetup {
    std::string Name;
    std::string ParentName;
    std::string Type;
    WAttributesMap Attributes;
    std::function<WActorComponent* (AActor* Owner)> CreateFunc;
};

struct FEventRuntimeBinding {
    std::string Tag;
    WAttributesMap Attributes;
    std::vector<WActionFactory> ActionFactories;
};

struct FTransitionRuntimeBinding
{
    std::string Target;
    WAttributesMap Attributes;
    std::vector<WActionFactory> ActionFactories;
};

struct FStateRuntimeSetup {
    std::string Name;
    std::string BaseName;
    std::vector<FEventRuntimeBinding> EventBindings;
    std::vector<FTransitionRuntimeBinding> TransitionBindings;
};

class FBlueprintAsset : public FAsset
{
	typedef FAsset Super;

protected:
	virtual bool LoadAsset(const std::wstring& FilePath) override;

private:
	bool SmartLoad(const std::wstring& FilePath, TArray<unsigned char>& RawBuffer);

	bool CheckIfNeedCompile(const std::wstring& Src, const std::wstring& Bin);

	bool OnCompile(const std::wstring& SrcPath, std::vector<unsigned char>& OutBuffer);

    void BindToActorFactory();

	void Serialize(FBinaryWriter& Writer, FXMLElement* RootElement);
	void SerializeAttributes(FBinaryWriter& Writer, FXMLElement* Element);
	void SerializeComponents(FBinaryWriter& Writer, FXMLElement* ComponentsElement);
	void SerializeComponent(FBinaryWriter& Writer, FXMLElement* ComponentElement);
	void SerializeStateMachine(FBinaryWriter& Writer, FXMLElement* StateMachineElement);
	void SerializeState(FBinaryWriter& Writer, FXMLElement* StateElement);
    void SerializeTransition(FBinaryWriter& Writer, FXMLElement* TransitionElement);
	void SerializeEvents(FBinaryWriter& Writer, FXMLElement* EventsElement);
	void SerializeEvent(FBinaryWriter& Writer, FXMLElement* EventElement);
    void SerializeActions(FBinaryWriter& Writer, FXMLElement* ActionsElement);
    void SerializeAction(FBinaryWriter& Writer, FXMLElement* ActionElement);

private:
    void Deserialize(FBinaryReader& Reader);
    void DeserializeAttributes(FBinaryReader& Reader, WAttributesMap& OutAttr);

    void DeserializeComponents(FBinaryReader& Reader, std::string ParentName = "");

    void DeserializeStateMachine(FBinaryReader& Reader);

    void DeserializeEvents(FBinaryReader& Reader, std::vector<FEventRuntimeBinding>& OutBindings);

    void DeserializeActions(FBinaryReader& Reader, std::vector<WActionFactory>& OutFactory);

public:
    // --- 컴파일된 런타임 데이터 ---
    std::string mParentClass;

    // 생성 순서와 계층 구조가 포함된 컴포넌트 생성 공장들
    std::vector<FComponentRuntimeSetup> mComponentSetups;

    // 스테이트 머신 런타임 데이터
    struct {
        bool bExist = false;
        std::string InitialState;
        std::unordered_map<std::string, FStateRuntimeSetup> States;
    } mRuntimeStateMachine;
};