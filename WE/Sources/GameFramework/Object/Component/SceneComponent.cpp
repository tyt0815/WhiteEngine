#include "SceneComponent.h"

void WSceneComponent::SetupAttachment(WSceneComponent* Parent)
{
	mParent = Parent;
	Parent->mChilds.push_back(this);
}

void WSceneComponent::UpdateRecursive()
{
	Update();

	for (WSceneComponent* Child : mChilds)
	{
		Child->UpdateRecursive();
	}
}

DirectX::XMFLOAT4 WSceneComponent::GetLocalQuatRotation()
{
	DirectX::XMVECTOR LocalEulerRadian = XMVectorSet(
		DirectX::XMConvertToRadians(mTransform.Rotation.x),
		DirectX::XMConvertToRadians(mTransform.Rotation.y),
		DirectX::XMConvertToRadians(mTransform.Rotation.z),
		0.0f
	);

	DirectX::XMVECTOR LocalQuat = XMQuaternionRotationRollPitchYawFromVector(LocalEulerRadian);
	DirectX::XMFLOAT4 LocalQuatRotation;
	DirectX::XMStoreFloat4(&LocalQuatRotation, LocalQuat);
	return LocalQuatRotation;
}

DirectX::XMFLOAT4 WSceneComponent::GetWorldQuatRotation()
{
	if (mParent)
	{
		DirectX::XMFLOAT4 ParentWorldQuatRotation = mParent->GetWorldQuatRotation();
		DirectX::XMFLOAT4 LocalQuatRotation = GetLocalQuatRotation();
		DirectX::XMVECTOR ParentWorldQuat = XMLoadFloat4(&ParentWorldQuatRotation);
		DirectX::XMVECTOR LocalQuat = DirectX::XMLoadFloat4(&LocalQuatRotation);
		DirectX::XMVECTOR WorldQuat = DirectX::XMQuaternionMultiply(LocalQuat, ParentWorldQuat);
		DirectX::XMFLOAT4 WorldQuatRotation;
		DirectX::XMStoreFloat4(&WorldQuatRotation, WorldQuat);
		return WorldQuatRotation;
	}
	else
	{
		return GetLocalQuatRotation();
	}
}

DirectX::XMFLOAT3 WSceneComponent::GetWorldLocation()
{
	DirectX::XMMATRIX WorldMat = DirectX::XMLoadFloat4x4(&mWorld);
	DirectX::XMVECTOR Pos = DirectX::XMLoadFloat3(&mTransform.Translation);
	Pos = DirectX::XMVectorSetW(Pos, 1.0f);
	
	DirectX::XMFLOAT3 WorldLocation;
	DirectX::XMStoreFloat3(&WorldLocation, DirectX::XMVector4Transform(Pos, WorldMat));
	return WorldLocation;
}

void WSceneComponent::SetLocalRotation(DirectX::XMFLOAT3 Rotation)
{
	mTransform.Rotation = Rotation;
	mTransform.Rotation.x = fmodf(mTransform.Rotation.x, 360.0f);
	mTransform.Rotation.y = fmodf(mTransform.Rotation.y, 360.0f);	
	mTransform.Rotation.z = fmodf(mTransform.Rotation.z, 360.0f);
	mbDirty = true;
}

void WSceneComponent::Update()
{
	UpdateWorldMatrix();
}

void WSceneComponent::UpdateWorldMatrix()
{
	if (mbDirty || (mParent && mParent->mbDirty))
	{
		XMFLOAT4X4 ParentWorld;
		if (mParent)
		{
			ParentWorld = mParent->GetWorldMatrix();
		}
		else
		{
			ParentWorld = FDXMath::Identity4x4();
		}
		XMMATRIX ParentWorldMat = XMLoadFloat4x4(&ParentWorld);
		XMFLOAT4X4 Local = mTransform.GetTransformFloat4x4();
		XMMATRIX LocalMat = XMLoadFloat4x4(&Local);
		XMStoreFloat4x4(&mWorld, ParentWorldMat * LocalMat);
		mbDirty = true;
	}
}
