#include "ViewCamera.h"

WViewCamera::WViewCamera()
{
}

void WViewCamera::RotateX(float Angle)
{
	Super::RotateX(Angle); 
	Transform.Rotation.x = UDXMath::Clamp(Transform.Rotation.x, -90.0f, 90.0f);
	bDirty = true;
}

void WViewCamera::UpdateViewMatrix()
{
	if (bDirty)
	{
		XMMATRIX RotationMatrix =
			XMMatrixRotationX(XMConvertToRadians(Transform.Rotation.x)) *
			XMMatrixRotationY(XMConvertToRadians(Transform.Rotation.y)) *
			XMMatrixRotationZ(XMConvertToRadians(Transform.Rotation.z));
		XMVECTOR P = XMLoadFloat3(&Transform.Translation);
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

		XMStoreFloat3(&Right, R);
		XMStoreFloat3(&Up, U);
		XMStoreFloat3(&Look, L);

		View(0, 0) = Right.x;
		View(1, 0) = Right.y;
		View(2, 0) = Right.z;
		View(3, 0) = x;

		View(0, 1) = Up.x;
		View(1, 1) = Up.y;
		View(2, 1) = Up.z;
		View(3, 1) = y;

		View(0, 2) = Look.x;
		View(1, 2) = Look.y;
		View(2, 2) = Look.z;
		View(3, 2) = z;

		View(0, 3) = 0.0f;
		View(1, 3) = 0.0f;
		View(2, 3) = 0.0f;
		View(3, 3) = 1.0f;

		bDirty = false;
	}
}

void WViewCamera::UpdateProjMatrix(float FovY, float AspectRatio, float NearZ, float FarZ)
{
	XMMATRIX P = XMMatrixPerspectiveFovLH(FovY, AspectRatio, NearZ, FarZ);
	XMStoreFloat4x4(&Proj, P);
}

void WViewCamera::Move(float X, float Y, float Z)
{
	// mPosition += d*mLook
	XMVECTOR Delta = XMVectorReplicate(X);
	XMVECTOR LookVector = XMLoadFloat3(&Look);
	XMVECTOR PositionVector = XMLoadFloat3(&Transform.Translation);
	PositionVector = XMVectorMultiplyAdd(Delta, LookVector, PositionVector);

	Delta = XMVectorReplicate(Y);
	XMVECTOR RightVector = XMLoadFloat3(&Right);
	PositionVector = XMVectorMultiplyAdd(Delta, RightVector, PositionVector);

	Delta = XMVectorReplicate(Z);
	XMVECTOR UpVector = XMLoadFloat3(&Up);
	PositionVector = XMVectorMultiplyAdd(Delta, UpVector, PositionVector);

	XMStoreFloat3(&Transform.Translation, PositionVector);

	bDirty = true;
}
