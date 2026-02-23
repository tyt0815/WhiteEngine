#include "PhysicsMovementComponent.h"

void WPhysicsMovementComponent::Tick(float DeltaSecond)
{
	constexpr float GravityConstant = 9.8f;
	XMVECTOR vWorldVelocity = XMLoadFloat3(&mVelocity);
	XMVECTOR vGravity = XMVectorSet(0.0f, -GravityConstant * mGravityScale * DeltaSecond, 0.0f, 0.0f);
	vWorldVelocity = XMVectorAdd(vWorldVelocity, vGravity);

	XMVECTOR vExternalAccel = XMLoadFloat3(&mExternalAcceleration);
	vWorldVelocity = XMVectorAdd(vWorldVelocity, vExternalAccel);
	mExternalAcceleration = { 0.f, 0.f, 0.f };

	if (mMaxSpeed >= 0.0f)
	{
		float SpeedSq = XMVectorGetX(XMVector3LengthSq(vWorldVelocity));
		if (SpeedSq > mMaxSpeed * mMaxSpeed)
		{
			vWorldVelocity = XMVectorScale(XMVector3Normalize(vWorldVelocity), mMaxSpeed);
		}
	}

	XMStoreFloat3(&mVelocity, vWorldVelocity);

	XMFLOAT3 Forward = GetWorldForwardVector();
	XMVECTOR vWorldForward = XMLoadFloat3(&Forward);

	if (XMVectorGetX(XMVector3LengthSq(vWorldVelocity)) > 0.01f)
	{
		XMVECTOR DirV = XMVector3Normalize(vWorldVelocity);

		if (mbOrientRotationToMovement)
		{
			float Radian = XMVectorGetX(XMVector3AngleBetweenNormals(vWorldForward, DirV));
			if (Radian > 0.0001f)
			{
				XMVECTOR AxisV = XMVector3Normalize(XMVector3Cross(vWorldForward, DirV));
				if (XMVector3Equal(AxisV, XMVectorZero()))
				{
					XMFLOAT3 Up = GetWorldUpVector();
					AxisV = XMLoadFloat3(&Up);
				}
				// 현재 방향과 속도 방향 사이의 차이만큼 회전 쿼터니언 생성
				XMVECTOR vQuat = XMQuaternionRotationAxis(AxisV, Radian);

				XMFLOAT4 CurrQuat = GetWorldQuatRotation();
				XMVECTOR CurrQuatV = XMLoadFloat4(&CurrQuat);

				XMFLOAT4 FinalQuat;
				XMStoreFloat4(&FinalQuat, XMQuaternionMultiply(CurrQuatV, vQuat));
				XMFLOAT3 FinalRotation = FDXMath::QuaternionToEuler(FinalQuat);

				SetWorldRotation(FinalRotation);
			}
		}
	}

	Super::Tick(DeltaSecond);
}

void WPhysicsMovementComponent::BeginComponent()
{
	Super::BeginComponent();

	XMFLOAT4 CurrQuat = GetWorldQuatRotation();
	XMVECTOR CurrQuatV = XMLoadFloat4(&CurrQuat);
	XMVECTOR LocalVelocityV = XMLoadFloat3(&mInitialVelocity);

	XMVECTOR WorldVelocityV = XMVector3Rotate(LocalVelocityV, CurrQuatV);
	XMStoreFloat3(&mVelocity, WorldVelocityV);
}

void WPhysicsMovementComponent::AddForce(const XMFLOAT3& Force)
{
	XMVECTOR CurrentAccel = XMLoadFloat3(&mExternalAcceleration);
	XMVECTOR NewForce = XMLoadFloat3(&Force);
	XMStoreFloat3(&mExternalAcceleration, XMVectorAdd(CurrentAccel, NewForce));
}
