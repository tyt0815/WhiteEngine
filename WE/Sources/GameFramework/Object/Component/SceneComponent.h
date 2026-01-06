#pragma once
#include "ActorComponent.h"
#include "DirectX/DXMath.h"

#include <vector>

class AActor;

class WSceneComponent : public WActorComponent
{
public:
	WSceneComponent() {};

	virtual ~WSceneComponent() noexcept override {};

public:
	void UpdateRecursive();

	DirectX::XMFLOAT4 GetLocalQuatRotation();

	DirectX::XMFLOAT4 GetWorldQuatRotation();

	DirectX::XMFLOAT3 GetWorldLocation();

	void SetupAttachment(WSceneComponent* Parent);

	void SetLocalRotation(DirectX::XMFLOAT3 Rotation);

protected:
	virtual void Update();
	bool mbDirty = true;

private:
	void UpdateWorldMatrix();
	FTransform mTransform;

	DirectX::XMFLOAT4X4 mWorld;

	DirectX::XMFLOAT4 mWorldQuat;

	WSceneComponent* mParent = nullptr;

	std::vector<WSceneComponent*> mChilds;

public:
	inline DirectX::XMFLOAT4X4 GetWorldMatrix()
	{
		return mWorld;
	}
	inline FTransform GetLocalTransform() const
	{
		return mTransform;
	}
	inline void SetLocalTransform(const FTransform& Transform)
	{
		mTransform = Transform;
		mbDirty = true;
	}
	inline DirectX::XMFLOAT3 GetLocalLocation() const
	{
		return mTransform.Translation;
	}
	inline void SetLocalLocation(DirectX::XMFLOAT3 Location)
	{
		mTransform.Translation = Location;
		mbDirty = true;
	}
	inline DirectX::XMFLOAT3 GetLocalRotation() const
	{
		return mTransform.Rotation;
	}
	inline DirectX::XMFLOAT3 GetLocalScale() const
	{
		return mTransform.Scale;
	}
	inline void SetLocalScale(DirectX::XMFLOAT3 Scale)
	{
		mTransform.Scale = Scale;
		mbDirty = true;
	}
	inline bool IsDirty() const
	{
		return mbDirty;
	}
};