#include "Actor.h"
#include "GameFramework/Object/Component/SceneComponent.h"
#include "GameFramework/Object/World/World.h"
#include "Asset/BlueprintAsset.h"

unsigned int g_ActorCounter = 0;

AActor::AActor():
	mActorCounter(++g_ActorCounter)
{
	TWeakPtr<WSceneComponent> DummyRoot = CreateComponent<WSceneComponent>()->GetWeakPtr<WSceneComponent>();
	SetRootComponent(DummyRoot);
}

void AActor::BeginPlay()
{
	Activate();
	BeginComponents();
}

void AActor::LoadBlueprint(const FBlueprintAsset* Blueprint)
{
	LoadBlueprintAttribute(Blueprint->mAttributes);

	WSceneComponent* RootComp = GetRootComponent();
	assert(RootComp);
	for (const auto& BlueprintComp : Blueprint->mAttachedComponents)
	{
		LoadBlueprintComponent_Internal(BlueprintComp.get(), RootComp);		
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

void AActor::RegisterToComponentFactory(const std::string& Type, std::function<WSceneComponent* (const FBlueprintAttributesMap&)> Lambda)
{
	assert(mComponentFactory.count(Type) == 0 && "Already registered component type");

	mComponentFactory[Type] = Lambda;
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

void AActor::LoadBlueprintComponent_Internal(FComponentNode* CompNode, WSceneComponent* Parent)
{
	assert(mComponentFactory.count(CompNode->Type) > 0 && "Unregistered component class");

	WSceneComponent* Comp = mComponentFactory[CompNode->Type](CompNode->Attributes);
	assert(Comp);
	Comp->SetupAttachment(Parent);
	if (CompNode->Attributes.count("Name") > 0) RegisterComponentByName(CompNode->Attributes["Name"], Comp);
	
	for (const auto ChildCompNode : CompNode->AttachedComponents)
	{
		LoadBlueprintComponent_Internal(ChildCompNode.get(), Comp);
	}
}

void AActor::RegisterComponentByName(const std::string& Name, WSceneComponent* Comp)
{
	assert(mBlueprintComponents.count(Name) == 0 && L"중복된 컴포넌트 이름 입니다.");

	mBlueprintComponents[Name] = Comp;
}
