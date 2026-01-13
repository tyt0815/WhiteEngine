#include "Actor.h"
#include "GameFramework/Object/Component/SceneComponent.h"
#include "GameFramework/Object/World/World.h"

AActor::AActor()
{
	
}

void AActor::BeginPlay()
{
	if (mbPhysicSimulate)
	{
		switch (mActorPhysicsShape)
		{
		case EPhysicsShape::EPS_Box:
			mBody = CreateBoxBody(mBoxPhysicsExtent, mObjectType);
			break;

		case EPhysicsShape::EPS_Sphere:
			mBody = CreateSphereBody(mSpherePhysicsRadius, mObjectType);
			break;

		default:
			break;
		}

		mBody->AddBody();

		mBody->SetTransform(GetActorTransform());
	}
}

void AActor::UpdatePhysics()
{
	if (mbPhysicSimulate)
	{
		SetActorTransform(mBody->GetTransform());
	}
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
	}

	mRootComponent = Component;
}

void AActor::SetActorTransform(FTransform Transform)
{
	mRootComponent->SetLocalTransform(Transform);

	UpdatePhysicsTransform();
}

void AActor::UpdatePhysicsTransform()
{
	if (mbPhysicSimulate)
	{
		mBody->SetTransform(GetActorTransform());
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
	GetWorld()->DestroyActor(this);
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