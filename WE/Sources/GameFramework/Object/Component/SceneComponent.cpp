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
		XMMATRIX LocalMat = mTransform.GetTransformMatrix();
		XMStoreFloat4x4(&mWorld, ParentWorldMat * LocalMat);
		mbDirty = true;
	}
}
