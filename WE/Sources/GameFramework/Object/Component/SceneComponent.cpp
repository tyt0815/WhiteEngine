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

void WSceneComponent::SetRotation(DirectX::XMFLOAT3 Rotation)
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
		XMFLOAT4X4 Local = mTransform.GetTransformMatrix();
		XMMATRIX LocalMat = XMLoadFloat4x4(&Local);
		XMStoreFloat4x4(&mWorld, ParentWorldMat * LocalMat);
		mbDirty = true;
	}
}
