#include "Actor.h"
#include "GameFramework/Object/Component/SceneComponent.h"

void AActor::Tick(float Delta)
{
}

void AActor::SetRootComponent(WSceneComponent* Component)
{
	if (mRootComponent == Component)
	{
		return;
	}
	if (mRootComponent != nullptr)
	{
		mRootComponent->SetupAttachment(Component);
		GetWObjectManager()->RemoveRootComponent(mRootComponentPoolId);
	}

	mRootComponent = Component;
	mRootComponentPoolId = GetWObjectManager()->RegisterRootComponent(mRootComponent);
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

void AActor::SetupComponent(WActorComponent* Component)
{
	Component->SetOwner(this);
}

void AActor::SetupSceneComponent(WSceneComponent* Component)
{
	if (mRootComponent)
	{
		Component->SetupAttachment(mRootComponent);
	}
	else
	{
		SetRootComponent(Component);
	}
}