#pragma once
#include "ActorComponent.h"
#include "DirectX/DXMath.h"

class AActor;

class WSceneComponent : public WActorComponent
{
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


	//inline XMMATRIX GetWorldMatrix() { return Transform.GetTransformMatrix(); }
	//inline FTransform GetTransform() const { return Transform; }
	//inline XMFLOAT3 GetScale() const { return Transform.Scale; }
	//inline XMFLOAT3 GetRotation() const { return Transform.Rotation; }
	//inline XMFLOAT3 GetTranslation() const { return Transform.Translation; }
	//inline void SetTransform(FTransform NewTransform) { Transform = NewTransform; }
	//inline virtual void SetTranslation(XMFLOAT3 Translation) { Transform.Translation = Translation; }
	//inline void SetRotation(XMFLOAT3 Rotation) { Transform.Rotation = Rotation; }
	//inline void SetScale(XMFLOAT3 Scale) { Transform.Scale = Scale; }
	//virtual void RotateX(float Angle) { Transform.Rotation.x += Angle; }
	//virtual void RotateY(float Angle) { Transform.Rotation.y += Angle; }
	//virtual void RotateZ(float Angle) { Transform.Rotation.z += Angle; }
};