#include "Actor.h"
#include "GameFramework/Object/Component/SceneComponent.h"
#include "GameFramework/Object/World/World.h"
#include "Component/PrimitiveComponent.h"

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

XMFLOAT3 AActor::GetFowardVector() const
{
	XMMATRIX RotationMatrix =
		XMMatrixRotationX(XMConvertToRadians(GetActorTransform().Rotation.x)) *
		XMMatrixRotationY(XMConvertToRadians(GetActorTransform().Rotation.y)) *
		XMMatrixRotationZ(XMConvertToRadians(GetActorTransform().Rotation.z));
	XMVECTOR L = XMVector3Transform({ 0.0f, 0.0f, 1.0f }, RotationMatrix);
	XMFLOAT3 Foward;
	XMStoreFloat3(&Foward, XMVector3Normalize(L));
	return Foward;
}

XMFLOAT3 AActor::GetRightVector() const
{
	XMMATRIX RotationMatrix =
		XMMatrixRotationX(XMConvertToRadians(GetActorTransform().Rotation.x)) *
		XMMatrixRotationY(XMConvertToRadians(GetActorTransform().Rotation.y)) *
		XMMatrixRotationZ(XMConvertToRadians(GetActorTransform().Rotation.z));
	XMVECTOR R = XMVector3Transform({ 1.0f, 0.0f, 0.0f }, RotationMatrix);
	XMFLOAT3 Right;
	XMStoreFloat3(&Right, R);
	return Right;
}

XMFLOAT3 AActor::GetUpVector() const
{
	XMMATRIX RotationMatrix =
		XMMatrixRotationX(XMConvertToRadians(GetActorTransform().Rotation.x)) *
		XMMatrixRotationY(XMConvertToRadians(GetActorTransform().Rotation.y)) *
		XMMatrixRotationZ(XMConvertToRadians(GetActorTransform().Rotation.z));
	XMVECTOR U = XMVector3Transform({ 0.0f, 1.0f, 0.0f }, RotationMatrix);
	XMFLOAT3 Up;
	XMStoreFloat3(&Up, U);
	return Up;
}

void AActor::Destroy()
{
	GetWorld()->DestroyActor(GetWeakPtr().lock());
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
	Deactivate();

	for (auto Comp : mAllComponents)
	{
		Comp->OnDestroy();
	}
}

void AActor::OnActivate()
{
	GetWorld()->EnqueueActorTick(this);
	for (auto& Comp : mAllComponents)
	{
		Comp->OnActivate();
	}
}

void AActor::OnDeactivate()
{
	GetWorld()->DequeueActorTick(this);
	for (auto& Comp : mAllComponents)
	{
		Comp->OnDeactivate();
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
		mAllComponents[i]->SetOwner(GetWeakPtr());
		mAllComponents[i]->BeginComponent();
	}
}