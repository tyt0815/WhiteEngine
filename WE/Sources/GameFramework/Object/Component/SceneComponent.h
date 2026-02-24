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
	void ActivateWithChild();

	void DeactivateWithChild();

	void UpdateWorldMatrix();

	DirectX::XMFLOAT4X4 GetWorldFloat4x4();

	DirectX::XMFLOAT4X4 GetInverseWorldFloat4x4();

	DirectX::XMMATRIX XM_CALLCONV GetWorldMatrix();

	DirectX::XMMATRIX XM_CALLCONV GetInverseWorldMatrix();

	DirectX::XMFLOAT4 GetLocalQuatRotation();

	DirectX::XMFLOAT4 GetWorldQuatRotation();

	DirectX::XMFLOAT3 GetWorldLocation();

	DirectX::XMFLOAT3 GetWorldRotation();

	DirectX::XMFLOAT3 GetWorldScale();

	XMFLOAT3 GetWorldForwardVector();

	XMFLOAT3 GetWorldRightVector();

	XMFLOAT3 GetWorldUpVector();

	XMFLOAT3 GetRelativeForwardVector();

	XMFLOAT3 GetRelativeRightVector();

	XMFLOAT3 GetRelativeUpVector();

	FTransform GetWorldTransform();

	void UpdateRecursive();

	void SetupAttachment(WSceneComponent* Parent);

	void SetRelativeLocation(DirectX::XMFLOAT3 Location);

	void SetRelativeRotation(DirectX::XMFLOAT3 Rotation);

	void SetRelativeScale(DirectX::XMFLOAT3 Scale);

	void SetRelativeTransform(const FTransform& Transform);

	void SetWorldLocation(XMFLOAT3 Location);

	void SetWorldRotation(XMFLOAT3 Rotation);

	void SetWorldScale(XMFLOAT3 Scale);

	void SetWorldTransform(FTransform Transform);

	void PropagateWorldFloat4Dirty(bool bForce = false);

	virtual void OnPropagateWorldFloat4Dirty();

	void AddWorldOffset(XMFLOAT3 WorldOffset);

protected:
	virtual void Update();

	virtual void OnSetTransform();

	virtual void PostSetupAttachment();

	bool mbWorldFloat4x4Dirty = true;

private:

	FTransform mTransform;

	DirectX::XMFLOAT4X4 mWorldFloat4x4;

	DirectX::XMFLOAT4X4 mInvWorldFloat4x4;

	TWeakPtr<WSceneComponent> mParent;

	std::vector<TWeakPtr<WSceneComponent>> mChilds;

public:
	inline FTransform GetRelativeTransform() const
	{
		return mTransform;
	}

	inline DirectX::XMFLOAT3 GetRelativeLocation() const
	{
		return mTransform.Translation;
	}

	inline DirectX::XMFLOAT3 GetRelativeRotation() const
	{
		return mTransform.Rotation;
	}

	inline DirectX::XMFLOAT3 GetRelativeScale() const
	{
		return mTransform.Scale;
	}

	inline bool IsDirty() const
	{
		return mbWorldFloat4x4Dirty;
	}

	__forceinline WSceneComponent* GetParent() const
	{
		return mParent.lock().get();
	}
};