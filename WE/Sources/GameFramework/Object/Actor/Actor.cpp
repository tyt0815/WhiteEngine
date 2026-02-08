#include "Actor.h"
#include "Component/SceneComponent.h"
#include "Component/StaticMeshComponent.h"
#include "World/World.h"
#include "Asset/BlueprintAsset.h"

unsigned int g_ActorCounter = 0;

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

			// 0. Static Mesh 에셋
			ApplyAttribute(Attributes, "Asset", [&](const std::string& v) {
				Comp->SetStaticMesh(v);
				});

			// 1. 위치 (Location)
			ApplyAttribute(Attributes, "Loc", ParseFloat3, [&](const XMFLOAT3& v) {
				Comp->SetLocalLocation(v);
				});

			// 2. 회전 (Rotation)
			ApplyAttribute(Attributes, "Rot", ParseFloat3, [&](const XMFLOAT3& v) {
				// 실수 방지: Rotation 전용 세터 호출
				Comp->SetLocalRotation(v);
				});

			// 3. 크기 (Scale)
			ApplyAttribute(Attributes, "Scale", ParseFloat3, [&](const XMFLOAT3& v) {
				// 실수 방지: Scale 전용 세터 호출
				Comp->SetLocalScale(v);
				});

			return Comp;
		});

	////////////////////////////////////////////////////////////////////////////////////////////////
	// WComponent End
	////////////////////////////////////////////////////////////////////////////////////////////////

	////////////////////////////////////////////////////////////////////////////////////////////////
	// WEvent
	////////////////////////////////////////////////////////////////////////////////////////////////

	mOnSpawnEvent = RegisterWEvent("OnSpawn");

	////////////////////////////////////////////////////////////////////////////////////////////////
	// WEvent End
	////////////////////////////////////////////////////////////////////////////////////////////////
}

void AActor::BeginPlay()
{
	Activate();
	BeginComponents();

	mOnSpawnEvent->Dispatch();
}

void AActor::LoadBlueprint(const FBlueprintAsset* Blueprint)
{
	LoadWAttributes(Blueprint->mAttributes);

	WSceneComponent* RootComp = GetRootComponent();
	assert(RootComp);
	for (const auto& BlueprintComp : Blueprint->mAttachedComponents)
	{
		LoadWComponent_Internal(BlueprintComp.get(), RootComp);		
	}

	for (const auto& EventInfo : Blueprint->mEvents)
	{
		if (mWEventsMap.count(EventInfo->Name) == 0)
		{
			ShowMessageBox("Invalid event name:\n" + EventInfo->Name);
			assert(false);
		}
		auto& Event = mWEventsMap[EventInfo->Name];

		for (const auto& ActionInfo : EventInfo->Actions)
		{
			auto ActionFactory = mWActionFactoryMap.find(ActionInfo->Name);
			if (ActionFactory == mWActionFactoryMap.end())
			{
				ShowMessageBox(std::string("Invalid Action name:\n") + EventInfo->Name + "::" + ActionInfo->Name);
				assert(false);
			}

			Event->AddAction(ActionFactory->second(ActionInfo->Attributes));
		}
	}
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

void AActor::Destroy()
{
	GetWorld()->DestroyActor(GetWeakPtr<AActor>().lock());
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

void AActor::RegisterWComponentFactory(const std::string& Type, WComponentFactory Lambda)
{
	assert(mWComponentFactoryMap.count(Type) == 0 && "Already registered component type");

	mWComponentFactoryMap[Type] = Lambda;
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

const AActor::WEvent* AActor::RegisterWEvent(const std::string& Name)
{
	if (mWEventsMap.count(Name) > 0)
	{
		ShowMessageBox("Already registered event:\n" + Name);
		assert(false);
	}

	mWEventsMap[Name] = MakeShared<WEvent>();
	return mWEventsMap[Name].get();
}

void AActor::LoadWComponent_Internal(FBlueprintComponentNode* CompNode, WSceneComponent* Parent)
{
	assert(mWComponentFactoryMap.count(CompNode->Type) > 0 && "Unregistered component class");

	WSceneComponent* Comp = mWComponentFactoryMap[CompNode->Type](CompNode->Attributes);
	assert(Comp);
	Comp->SetupAttachment(Parent);
	if (CompNode->Attributes.count("Name") > 0) RegisterWComponent(CompNode->Attributes["Name"], Comp);
	
	for (const auto ChildCompNode : CompNode->AttachedComponents)
	{
		LoadWComponent_Internal(ChildCompNode.get(), Comp);
	}
}

void AActor::RegisterWActionFactory(const std::string Name, WActionFactoryFunc Lambda)
{
	mWActionFactoryMap[Name] = Lambda;
}

void AActor::RegisterWComponent(const std::string& Name, WSceneComponent* Comp)
{
	assert(mWComponentsMap.count(Name) == 0 && L"중복된 컴포넌트 이름 입니다.");

	mWComponentsMap[Name] = Comp;
}

void ReportParseError(const std::string& Type, const std::string& WrongValue)
{
	std::string ErrorMsg = "Invalid " + Type + " format: " + WrongValue;
	ShowMessageBox(ErrorMsg);
	assert(false && "Check the XML attribute format!");
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

