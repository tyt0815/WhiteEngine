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
	BeginComponents();
}

void AActor::Tick_PrePhysics(float Delta)
{
	TickComponents_PrePhysics(Delta);
}

void AActor::Tick_PostPhysics(float Delta)
{
	TickComponents_PostPhysics(Delta);
}

void AActor::UpdateComponentsToPhysics()
{
	for (auto& CompWeak : mAllPhysicsComponents)
	{
		if (auto Comp = CompWeak.lock())
		{
			Comp->UpdateToPhysics();
		}
	}
}

void AActor::UpdateComponentsFromPhysics()
{
	for (auto& CompWeak : mAllPhysicsComponents)
	{
		if (auto Comp = CompWeak.lock())
		{
			Comp->UpdateFromPhysics();
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
	XMStoreFloat3(&Foward, L);
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

void AActor::BeginComponents()
{
	for (int i = 0; i < mAllComponents.size(); ++i)
	{
		mAllComponents[i]->SetOwner(GetWeakPtr());
		mAllComponents[i]->BeginComponent();
	}
}

void AActor::TickComponents_PrePhysics(float Delta)
{
	for (int i = 0; i < mAllComponents.size(); ++i)
	{
		mAllComponents[i]->TickComponent_PrePhysics(Delta);
	}
}

void AActor::TickComponents_PostPhysics(float Delta)
{
	for (int i = 0; i < mAllComponents.size(); ++i)
	{
		mAllComponents[i]->TickComponent_PostPhysics(Delta);
	}
}