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
}

void AActor::BeginPlay()
{
	Activate();
	BeginComponents();
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
			OldRoot->SetupAttachment(NewRoot);
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

void AActor::LoadBlueprint(const std::wstring& Name)
{
	FBlueprintAsset* Asset = FAssetManager::GetAsset<FBlueprintAsset>(Name);

	const auto& FloatMap = Asset->mFloatMap;
	for (const auto& KeyValue : FloatMap)
	{
		const std::string& Key = KeyValue.first;
		const float Value = KeyValue.second;

		if (mBlueprintMap.count(Key))
		{
			*(reinterpret_cast<float*>(mBlueprintMap[Key])) = Value;
		}
	}

	const auto& StaticMeshComponentMap = Asset->mStaticMeshComponentMap;
	for (const auto& KeyValue : StaticMeshComponentMap)
	{
		const std::string& Key = KeyValue.first;
		const auto& Info = KeyValue.second;

		if (mBlueprintMap.count(Key))
		{
			WStaticMeshComponent* StaticMeshComp = reinterpret_cast<WStaticMeshComponent*>(mBlueprintMap[Key]);
			StaticMeshComp->SetStaticMesh(FStaticMeshManager::GetStaticMesh("SM_MetalRing"));
		}
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
		mAllComponents[i]->SetOwner(GetWeakPtr<AActor>());
		mAllComponents[i]->BeginComponent();
	}
}