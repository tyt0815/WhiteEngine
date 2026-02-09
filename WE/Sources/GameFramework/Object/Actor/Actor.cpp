#include "Actor.h"
#include "Component/SceneComponent.h"
#include "Component/StaticMeshComponent.h"
#include "Component/SplineComponent.h"
#include "Component/ObjectAnimComponent.h"
#include "World/World.h"
#include "Asset/BlueprintAsset.h"

unsigned int g_ActorCounter = 0;

void ReportParseError(const std::string& Type, const std::string& WrongValue)
{
	std::string ErrorMsg = "Invalid " + Type + " format: " + WrongValue;
	ShowMessageBox(ErrorMsg);
	assert(false && "Check the XML attribute format!");
}

void ApplySceneComponentDefaultAttributes(WSceneComponent* Comp, const std::unordered_map<std::string, std::string>& Attributes)
{
	ApplyAttribute(Attributes, "Loc", ParseFloat3, [&](const XMFLOAT3& v) {
		Comp->SetLocalLocation(v);
		});

	ApplyAttribute(Attributes, "Rot", ParseFloat3, [&](const XMFLOAT3& v) {
		// 실수 방지: Rotation 전용 세터 호출
		Comp->SetLocalRotation(v);
		});

	ApplyAttribute(Attributes, "Scale", ParseFloat3, [&](const XMFLOAT3& v) {
		// 실수 방지: Scale 전용 세터 호출
		Comp->SetLocalScale(v);
		});
}

AActor::AActor():
	mActorCounter(++g_ActorCounter)
{
	TWeakPtr<WSceneComponent> DummyRoot = CreateComponent<WSceneComponent>()->GetWeakPtr<WSceneComponent>();

	SetRootComponent(DummyRoot);

	////////////////////////////////////////////////////////////////////////////////////////////////
	// WComponent
	////////////////////////////////////////////////////////////////////////////////////////////////
	

	RegisterWComponentFactory("Mesh", [this](const WAttributesMap& Attributes)
		{
			WStaticMeshComponent* Comp = this->CreateComponent<WStaticMeshComponent>();

			ApplySceneComponentDefaultAttributes(Comp, Attributes);

			// Static Mesh 에셋
			ApplyAttribute(Attributes, "Asset", [&](const std::string& v) {
				Comp->SetStaticMesh(v);
				});

			return Comp;
		});

	RegisterWComponentFactory("Spline", [this](const WAttributesMap& Attributes)
		{
			WSplineComponent* Comp = this->CreateComponent<WSplineComponent>();

			ApplySceneComponentDefaultAttributes(Comp, Attributes);

			ApplyAttribute(Attributes, "Asset", [&](const std::string& v) {
				Comp->LoadSplineFromAsset(v);
				});

			return Comp;
		});

	////////////////////////////////////////////////////////////////////////////////////////////////
	// WComponent End
	////////////////////////////////////////////////////////////////////////////////////////////////

	////////////////////////////////////////////////////////////////////////////////////////////////
	// WAction
	////////////////////////////////////////////////////////////////////////////////////////////////

	RegisterWActionFactory("Event", [this](const WAttributesMap& Attributes) {
		std::string Name = Attributes.at("Name");
		return [this, Name]() { this->mWEventsMap[Name]->Dispatch(); };
		});	

	RegisterWActionFactory("Set", [this](const WAttributesMap& Attributes) {

		std::string Name = Attributes.at("Name");
		std::string RawValue = Attributes.at("Value");

		auto it = mWPropertiesMap.find(Name);
		WVariantValue ParsedValue;

		std::visit([&](auto&& Arg) {
			using T = std::remove_pointer_t<std::decay_t<decltype(Arg)>>;

			ParsedValue = WPropertyTrait<T>::Parse(RawValue);
			}, it->second);

		return [this, Name, ParsedValue]() {
			auto Property = mWPropertiesMap[Name];

			// 실행 시점에는 단순 값 대입만 발생 (파싱 X, 매우 빠름)
			std::visit([](auto&& TargetPtr, auto&& SourceValue) {
				using TargetType = std::remove_pointer_t<std::decay_t<decltype(TargetPtr)>>;
				using SourceType = std::decay_t<decltype(SourceValue)>;

				if constexpr (std::is_same_v<TargetType, SourceType>) {
					if (TargetPtr) *TargetPtr = SourceValue;
				}
				}, Property, ParsedValue);
		};
		});

	RegisterWActionFactory("Destroy", [this](const WAttributesMap& Attributes) {
		return [this]() { this->Destroy(); };
		});

	RegisterWActionFactory("SpawnActor", [this](const WAttributesMap& Attributes)
		{
			std::string Name = Attributes.at("Name");
			
			FTransform LocalTransform;
			ExtractAttribute(Attributes, "Loc", LocalTransform.Translation);
			ExtractAttribute(Attributes, "Rot", LocalTransform.Rotation);
			ExtractAttribute(Attributes, "Scale", LocalTransform.Scale);

			return [=]()
			{
				FActorSpawnParameter Param;
				XMMATRIX W = this->GetWorldMatrix();
				XMMATRIX L = LocalTransform.GetTransformMatrix();
				Param.Transform.SetByTransformMatrix(L * W);
				GetWorld()->SpawnActorByFactory<AActor>(Name, Param);
			};
		});

	////////////////////////////////////////////////////////////////////////////////////////////////
	// WAction End
	////////////////////////////////////////////////////////////////////////////////////////////////

	////////////////////////////////////////////////////////////////////////////////////////////////
	// WEvent
	////////////////////////////////////////////////////////////////////////////////////////////////

	mOnSpawnEvent = RegisterWEvent("OnSpawn");

	mOnDestroyEvent = RegisterWEvent("OnDestroy");

	RegisterWEvent("OnTime", [=](const WEvent* Event, const WAttributesMap& Attributes) {
		TSharedPtr<FOnTimeEvent> OnTimeEvent = MakeShared<FOnTimeEvent>();
		OnTimeEvent->Event = Event;
		ExtractAttribute(Attributes, "Time", OnTimeEvent->Time);

		mOnTimeEvents.push_back(std::move(OnTimeEvent));
		});

	////////////////////////////////////////////////////////////////////////////////////////////////
	// WEvent End
	////////////////////////////////////////////////////////////////////////////////////////////////
}

void AActor::Tick(float DeltaSecond)
{
	Super::Tick(DeltaSecond);

	mElapsedTime += DeltaSecond;

	int NumOnTimeEvent = (int)mOnTimeEvents.size();
	while (mOnTimeEventIndex < NumOnTimeEvent && mOnTimeEvents[mOnTimeEventIndex]->Time <= mElapsedTime)
	{
		mOnTimeEvents[mOnTimeEventIndex++]->Event->Dispatch();
	}

}

void AActor::Destroy()
{
	if (!IsPendingKill())
	{
		GetWorld()->DestroyActor(GetWeakPtr<AActor>().lock());
		mOnDestroyEvent->Dispatch();
	}
}

void AActor::Activate()
{
	GetWorld()->ActivateActor(this);
}

void AActor::Deactivate()
{
	GetWorld()->DeactivateActor(this);
}

void AActor::OnDestroy()
{
	Super::OnDestroy();

	for (auto Comp : mAllComponents)
	{
		Comp->OnDestroy();
	}
}

void AActor::OnActivate()
{
	Super::OnActivate();
	for (auto& Comp : mAllComponents)
	{
		Comp->OnActivate();
	}
}

void AActor::OnDeactivate()
{
	for (auto& Comp : mAllComponents)
	{
		Comp->OnDeactivate();
	}
	Super::OnDeactivate();
}

void AActor::BeginPlay()
{
	Activate();
	BeginComponents();

	std::sort(mOnTimeEvents.begin(), mOnTimeEvents.end(), [](const TSharedPtr<FOnTimeEvent>& A, const TSharedPtr<FOnTimeEvent>& B) { return A->Time < B->Time; });

	mOnSpawnEvent->Dispatch();

}

XMFLOAT3 AActor::GetForwardVector() const
{
	XMMATRIX RotationMatrix = GetActorTransform().GetRotationMatrix();
	XMVECTOR L = XMVector3Transform({ 0.0f, 0.0f, 1.0f }, RotationMatrix);
	XMFLOAT3 Foward;
	XMStoreFloat3(&Foward, XMVector3Normalize(L));
	return Foward;
}

XMFLOAT3 AActor::GetRightVector() const
{
	XMMATRIX RotationMatrix = GetActorTransform().GetRotationMatrix();
	XMVECTOR R = XMVector3Transform({ 1.0f, 0.0f, 0.0f }, RotationMatrix);
	XMFLOAT3 Right;
	XMStoreFloat3(&Right, R);
	return Right;
}

XMFLOAT3 AActor::GetUpVector() const
{
	XMMATRIX RotationMatrix = GetActorTransform().GetRotationMatrix();
	XMVECTOR U = XMVector3Transform({ 0.0f, 1.0f, 0.0f }, RotationMatrix);
	XMFLOAT3 Up;
	XMStoreFloat3(&Up, U);
	return Up;
}

XMFLOAT4 AActor::GetActorQuaternion()
{
	return mRootComponent.lock()->GetWorldQuatRotation();
}

void AActor::SetRootComponent(TWeakPtr<WSceneComponent> Component)
{
	if (!mRootComponent.expired() && !Component.expired())
	{
		TSharedPtr<WSceneComponent> OldRoot = mRootComponent.lock();
		TSharedPtr<WSceneComponent> NewRoot = Component.lock();
		if (OldRoot.get() == NewRoot.get())
		{
			return;
		}
		else
		{
			OldRoot->SetupAttachment(NewRoot.get());
		}
	}

	mRootComponent = Component;
}

void AActor::SetActorTransform(FTransform Transform)
{
	if (auto Root = mRootComponent.lock())
	{
		Root->SetLocalTransform(Transform);
	}
}

void AActor::OnCreateComponent(WActorComponent* Comp)
{
	Comp->mOwner = this;
	if (WSceneComponent* SceneComp = dynamic_cast<WSceneComponent*>(Comp))
	{
		mAllSceneComponent.emplace_back(SceneComp->GetWeakPtr<WSceneComponent>());
		if (WPhysicsComponent* PhysicsComp = dynamic_cast<WPhysicsComponent*>(SceneComp))
		{
			mAllPhysicsComponents.emplace_back(PhysicsComp->GetWeakPtr<WPhysicsComponent>());
		}
	}
	else
	{
		mAllNoneSceneComponent.emplace_back(Comp->GetWeakPtr<WActorComponent>());
	}
}

void AActor::UpdateRecursive()
{
	if (auto Root = mRootComponent.lock())
	{
		Root->UpdateRecursive();
	}
}

void AActor::BeginComponents()
{
	for (int i = 0; i < mAllComponents.size(); ++i)
	{
		mAllComponents[i]->BeginComponent();
	}
}

void AActor::LoadBlueprint(const FBlueprintAsset* Blueprint)
{
	LoadWConfigs(Blueprint->mConfigs);

	WSceneComponent* RootComp = GetRootComponent();
	assert(RootComp);
	for (const auto& BlueprintComp : Blueprint->mAttachedComponents)
	{
		LoadWComponent_Internal(BlueprintComp.get(), RootComp);
	}

	LoadWEvents(Blueprint->mEvents);
}

void AActor::LoadWEvent(AActor::WEvent* Event, FBlueprintEventNode* EventNode)
{
	Event->Initializer(Event, EventNode->Attributes);

	for (const auto& ActionInfo : EventNode->Actions)
	{
		auto ActionFactory = mWActionFactoryMap.find(ActionInfo->Name);
		if (ActionFactory == mWActionFactoryMap.end())
		{
			ShowMessageBox(std::string("Invalid Action name:\n") + Event->Name + "::" + ActionInfo->Name);
			assert(false);
		}

		Event->AddAction(ActionFactory->second(ActionInfo->Attributes));
	}
}

void AActor::RegisterWComponentFactory(const std::string& Type, WComponentFactory Lambda)
{
	assert(mWComponentFactoryMap.count(Type) == 0 && "Already registered component type");

	mWComponentFactoryMap[Type] = Lambda;
}

void AActor::RegisterWActionFactory(const std::string Name, WActionFactoryFunc Lambda)
{
	mWActionFactoryMap[Name] = Lambda;
}

const AActor::WEvent* AActor::RegisterWEvent(const std::string& Name)
{
	WEvent* Event = RegisterWEvent_Internal(Name);
	Event->Initializer = [](auto&&, auto&&) {};
	return Event;
}

void AActor::RegisterWEvent(const std::string& Name, std::function<void(const WEvent* Event, const WAttributesMap&)> InitializerFunc)
{
	WEvent* Event = RegisterWEvent_Internal(Name);

	Event->Initializer = InitializerFunc;
}

void AActor::SetWProperty(const std::string& Name, WVariantValue Value)
{
	WProperty Property = mWPropertiesMap[Name];

	// 실행 시점에는 단순 값 대입만 발생 (파싱 X, 매우 빠름)
	std::visit([](auto&& TargetPtr, auto&& SourceValue) {
		using TargetType = std::remove_pointer_t<std::decay_t<decltype(TargetPtr)>>;
		using SourceType = std::decay_t<decltype(SourceValue)>;

		if constexpr (std::is_same_v<TargetType, SourceType>) {
			if (TargetPtr) *TargetPtr = SourceValue;
		}
		}, Property, Value);
}

void AActor::LoadWComponent_Internal(FBlueprintComponentNode* CompNode, WSceneComponent* Parent)
{
	assert(mWComponentFactoryMap.count(CompNode->Type) > 0 && "Unregistered component class");

	WSceneComponent* Comp = mWComponentFactoryMap[CompNode->Type](CompNode->Attributes);
	assert(Comp);
	Comp->SetupAttachment(Parent);
	if (CompNode->Attributes.count("Name") > 0) RegisterWComponent(CompNode->Attributes["Name"], Comp);

	OnLoadWComponent(CompNode, Comp);
	
	for (const auto ChildCompNode : CompNode->AttachedComponents)
	{
		LoadWComponent_Internal(ChildCompNode.get(), Comp);
	}
}

void AActor::LoadWEvents(const TArray<TSharedPtr<FBlueprintEventNode>>& Events)
{
	for (const auto& EventInfo : Events)
	{
		WEvent* Event = nullptr;
		if (EventInfo->Name.substr(0, 2) == "On")
		{
			if (mWEventsMap.count(EventInfo->Name) == 0)
			{
				ShowMessageBox("Invalid event name:\n" + EventInfo->Name);
				assert(false);
			}
		}
		else
		{
			if (mWEventsMap.count(EventInfo->Name) > 0)
			{
				ShowMessageBox("Already registered custom event:\n" + EventInfo->Name);
				assert(false);
			}

			RegisterWEvent(EventInfo->Name);
		}
		if (Event == nullptr)
		{
			Event = mWEventsMap[EventInfo->Name].get();
		}

		LoadWEvent(Event, EventInfo.get());
	}
}

void AActor::RegisterWComponent(const std::string& Name, WSceneComponent* Comp)
{
	assert(mWComponentsMap.count(Name) == 0 && L"중복된 컴포넌트 이름 입니다.");

	mWComponentsMap[Name] = Comp;
}

AActor::WEvent* AActor::RegisterWEvent_Internal(const std::string& Name)
{
	if (mWEventsMap.count(Name) > 0)
	{
		ShowMessageBox("Already registered event:\n" + Name);
		assert(false);
	}

	TSharedPtr<WEvent> Event = MakeShared<WEvent>();
	Event->Name = Name;
	mWEventsMap[Name] = Event;
	return Event.get();
}

// 1. Float3 파서
XMFLOAT3 ParseFloat3(const std::string& String)
{
	XMFLOAT3 Float3 = { 0.f, 0.f, 0.f };
	// 유저님이 작성하신 괄호 패턴 유지
	int Result = sscanf_s(String.c_str(), "(%f, %f, %f)", &Float3.x, &Float3.y, &Float3.z);

	if (Result < 3)
	{
		ReportParseError("Float3 (x, y, z)", String);
	}
	return Float3;
}

// 2. Float 파서
float ParseFloat(const std::string& String)
{
	try {
		return std::stof(String);
	}
	catch (...) {
		ReportParseError("Float", String);
		return 0.0f;
	}
}

// 3. Int 파서
int ParseInt(const std::string& String)
{
	try {
		return std::stoi(String);
	}
	catch (...) {
		ReportParseError("Int", String);
		return 0;
	}
}

// 4. Bool 파서
bool ParseBool(const std::string& String)
{
	std::string LowerStr = String;
	std::transform(LowerStr.begin(), LowerStr.end(), LowerStr.begin(), ::tolower);

	if (LowerStr == "true" || LowerStr == "1") return true;
	if (LowerStr == "false" || LowerStr == "0") return false;

	ReportParseError("Bool (true/false/1/0)", String);
	return false;
}