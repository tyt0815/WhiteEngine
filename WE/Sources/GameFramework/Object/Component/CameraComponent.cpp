#include "CameraComponent.h"
#include "GameFramework/Object/Pawn/Pawn.h"

extern float gAspectRatio;

WCameraComponent::WCameraComponent()
{
}

void WCameraComponent::SetOwner(TWeakPtr<AActor> Owner)
{
	Super::SetOwner(Owner);

	if (TSharedPtr<APawn> OwnerPawn = Cast<APawn>(Owner.lock()))
	{
		OwnerPawn->SetCameraComponent(GetWeakPtr<WCameraComponent>());
	}
}

DirectX::XMFLOAT4X4 WCameraComponent::GetViewMatrix()
{
	UpdateViewMatrix();
	return mView;
}

DirectX::XMFLOAT4X4 WCameraComponent::GetProjMatrix()
{
	UpdateProjMatrix();
	return mProj;
}

void WCameraComponent::OnSetTransform()
{
	mbViewFloat4x4Dirty = true;
	Super::OnSetTransform();
}

void WCameraComponent::UpdateViewMatrix()
{
	if (mbViewFloat4x4Dirty)
	{
		XMMATRIX RotationMatrix = GetLocalTransform().GetRotationMatrix();
		XMVECTOR P = GetLocalTransform().GetTranslationVector();
		XMVECTOR L = XMVector3Transform({ 0.0f, 0.0f, 1.0f }, RotationMatrix);
		XMVECTOR U = XMVector3Transform({ 0.0f, 1.0f, 0.0f }, RotationMatrix);
		XMVECTOR R = XMVector3Transform({ 1.0f, 0.0f, 0.0f }, RotationMatrix);


		// Keep camera's axes orthogonal to each other and of unit length.
		L = XMVector3Normalize(L);
		U = XMVector3Normalize(XMVector3Cross(L, R));

		// U, L already ortho-normal, so no need to normalize cross product.
		R = XMVector3Cross(U, L);

		// Fill in the view matrix entries.
		float x = -XMVectorGetX(XMVector3Dot(P, R));
		float y = -XMVectorGetX(XMVector3Dot(P, U));
		float z = -XMVectorGetX(XMVector3Dot(P, L));

		XMStoreFloat3(&mRight, R);
		XMStoreFloat3(&mUp, U);
		XMStoreFloat3(&mLook, L);

		mView(0, 0) = mRight.x;
		mView(1, 0) = mRight.y;
		mView(2, 0) = mRight.z;
		mView(3, 0) = x;

		mView(0, 1) = mUp.x;
		mView(1, 1) = mUp.y;
		mView(2, 1) = mUp.z;
		mView(3, 1) = y;

		mView(0, 2) = mLook.x;
		mView(1, 2) = mLook.y;
		mView(2, 2) = mLook.z;
		mView(3, 2) = z;

		mView(0, 3) = 0.0f;
		mView(1, 3) = 0.0f;
		mView(2, 3) = 0.0f;
		mView(3, 3) = 1.0f;

		mbViewFloat4x4Dirty = false;
	}
}

void WCameraComponent::UpdateProjMatrix()
{
	if (mbProjFloat4x4Dirty || gAspectRatio != mAspectRatio)
	{
		mAspectRatio = gAspectRatio;
		XMMATRIX P = XMMatrixPerspectiveFovLH(mFov * FDXMath::Pi / 180, mAspectRatio, mNearZ + 1, mFarZ);
		XMStoreFloat4x4(&mProj, P);
		mbProjFloat4x4Dirty = false;
	}
}
