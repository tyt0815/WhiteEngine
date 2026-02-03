#pragma once
#include "ActorComponent.h"
#include "DirectX/DXMath.h"

#include <vector>

class AActor;

class WSceneComponent : public WActorComponent
{
	typedef WActorComponent Super;
public:
	WSceneComponent();

	virtual ~WSceneComponent() noexcept override {};

protected:

public:
	void UpdateWorldMatrix();

	DirectX::XMFLOAT4X4 GetWorldFloat4x4();

	DirectX::XMFLOAT4X4 GetInverseWorldFloat4x4();

	DirectX::XMMATRIX XM_CALLCONV GetWorldMatrix();

	DirectX::XMMATRIX XM_CALLCONV GetInverseWorldMatrix();

	DirectX::XMFLOAT4 GetLocalQuatRotation();

	DirectX::XMFLOAT4 GetWorldQuatRotation();

	DirectX::XMFLOAT3 GetWorldLocation();

	FTransform GetWorldTransform();

	void UpdateRecursive();

	void SetupAttachment(WSceneComponent* Parent);

	void SetLocalRotation(DirectX::XMFLOAT3 Rotation);

	void SetLocalTransform(const FTransform& Transform);

	void SetLocalLocation(DirectX::XMFLOAT3 Location);

	void SetLocalScale(DirectX::XMFLOAT3 Scale);

	void SetWorldTransform(FTransform Transform);

protected:
	virtual void Update();

	virtual void OnSetTransform();

	bool mbWorldFloat4x4Dirty = true;

private:

	FTransform mTransform;

	DirectX::XMFLOAT4X4 mWorldFloat4x4;

	DirectX::XMFLOAT4X4 mInvWorldFloat4x4;

	TWeakPtr<WSceneComponent> mParent;

	std::vector<TWeakPtr<WSceneComponent>> mChilds;

public:
	inline FTransform GetLocalTransform() const
	{
		return mTransform;
	}
	inline DirectX::XMFLOAT3 GetLocalLocation() const
	{
		return mTransform.Translation;
	}

	inline DirectX::XMFLOAT3 GetLocalRotation() const
	{
		return mTransform.Rotation;
	}

	inline DirectX::XMFLOAT3 GetLocalScale() const
	{
		return mTransform.Scale;
	}

	inline bool IsDirty() const
	{
		return mbWorldFloat4x4Dirty;
	}
};