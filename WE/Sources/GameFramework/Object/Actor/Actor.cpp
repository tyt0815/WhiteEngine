#include "Actor.h"
#include "GameFramework/Object/Component/SceneComponent.h"
#include "GameFramework/Object/World/World.h"
#include "Component/PrimitiveComponent.h"
#include "Component/StaticMeshComponent.h"
#include "Asset/AssetManager.h"
#include "Asset/BlueprintAsset.h"

AActor::AActor()
{
	TWeakPtr<WSceneComponent> DummyRoot = CreateComponent<WSceneComponent>();
	SetRootComponent(DummyRoot);

	BEGIN_WFUNCTION(SpawnActor)
	{
		FActorSpawnParameter SpawnParam;
		std::string Class = "AActor";
		for (const auto& Param : Params)
		{
			if (Param->Name == "Class")
			{
				Class = Param->Get<std::string>();
			}
			else if (Param->Name == "Location")
			{
				SpawnParam.Transform.Translation = Param->Get<XMFLOAT3>();
			}
			else if (Param->Name == "Rotation")
			{
				SpawnParam.Transform.Rotation = Param->Get<XMFLOAT3>();
			}
			else if (Param->Name == "Scale")
			{
				SpawnParam.Transform.Scale = Param->Get<XMFLOAT3>();
			}
			else if (Param->Name == "Transform")
			{
				SpawnParam.Transform = Param->Get<FTransform>();
			}
			else
			{
				assert(false && "Invalid parameter name");
			}
		}
		return GetWorld()->SpawnActorByFactory<AActor>(Params[0]->Get<std::string>(), SpawnParam).lock();
	}
	END_WFUNCTION;

	REGISTER_WFUNC_RET_0(GetRootComponent, WSceneComponent*);

	REGISTER_WFUNC_RET_0(GetActorLocation, XMFLOAT3);
	REGISTER_WFUNC_RET_0(GetActorRotation, XMFLOAT3);
	REGISTER_WFUNC_RET_0(GetActorScale, XMFLOAT3);
	REGISTER_WFUNC_RET_0(GetActorTransform, FTransform);

	mBeginPlayEvent = RegisterEvent("BeginPlay");
}

void AActor::BeginPlay()
{
	Activate();
	BeginComponents();

	mBeginPlayEvent->Dispatch();
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

	RegisterWPropertySafe("RootComponent", mRootComponent.lock().get());
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

void AActor::LoadBlueprint(BlueprintAsset::FActorNode* RootNode)
{
	LoadWProperties(RootNode->Properties);
	LoadWVariables(RootNode->Variables);
	LoadEvents(this, RootNode->Events);

	for (const auto& CompNode : RootNode->AttachedComponents)
	{
		WActorComponent* Comp = GetWPropertyPtrSafe<WActorComponent>(CompNode->Name);
		if (!Comp)
		{
			Comp = CreateComponentByFactory<WActorComponent>(CompNode->ComponentNode.ParentClass).lock().get();
			if (WSceneComponent* SceneComp = dynamic_cast<WSceneComponent*>(Comp))
			{
				SceneComp->SetupAttachment(GetRootComponent());
			}
			RegisterWProperty(CompNode->Name, Comp);
		}
		Comp->LoadBlueprint(this ,&CompNode->ComponentNode);
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
