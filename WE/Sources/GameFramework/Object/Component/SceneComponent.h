#pragma once
#include "ActorComponent.h"
#include "DirectX/DXMath.h"

class AActor;

class WSceneComponent : public WActorComponent
{
public:
	void SetupAttachment(WSceneComponent* Parent);
	void UpdateRecursive();

protected:
	virtual void Update();
	bool mbDirty = true;

private:
	void UpdateWorldMatrix();
	FTransform mTransform;
	DirectX::XMFLOAT4X4 mWorld;
	WSceneComponent* mParent = nullptr;
	std::vector<WSceneComponent*> mChilds;

public:
	inline DirectX::XMFLOAT4X4 GetWorldMatrix()
	{
		return mWorld;
	}
	inline FTransform GetTransform() const
	{
		return mTransform;
	}
	inline void SetTransform(const FTransform& Transform)
	{
		mTransform = Transform;
		mbDirty = true;
	}
	inline DirectX::XMFLOAT3 GetLocation() const
	{
		return mTransform.Translation;
	}
	inline void SetLocation(DirectX::XMFLOAT3 Location)
	{
		mTransform.Translation = Location;
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