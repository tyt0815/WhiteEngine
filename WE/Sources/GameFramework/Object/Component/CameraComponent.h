#pragma once
#include "SceneComponent.h"

class WCameraComponent : public WSceneComponent
{
	typedef WSceneComponent Super;
public:
	WCameraComponent();
	virtual void SetOwner(AActor* Owner) override;

protected:
	virtual void Update() override;

private:
	void UpdateViewMatrix();
	void UpdateProjMatrix();
	XMFLOAT4X4 mView = FDXMath::Identity4x4();
	XMFLOAT4X4 mProj = FDXMath::Identity4x4();
	XMFLOAT3 mRight;
	XMFLOAT3 mUp;
	XMFLOAT3 mLook;
	float mFov = 90;
	float mNearZ = 1;
	float mFarZ = 1000;
	float mAspectRatio = 1;
	bool mbProjDirty = true;
public:
	inline DirectX::XMFLOAT4X4 GetViewMatrix() const
	{
		return mView;
	}
	inline DirectX::XMFLOAT4X4 GetProjMatrix() const
	{
		return mProj;
	}
	inline float GetNearZ() const
	{
		return mNearZ;
	}
	inline float GetFarZ() const
	{
		return mFarZ;
	}

	//virtual void SetTranslation(XMFLOAT3 Translation) override { Super::SetTranslation(Translation); bDirty = true; }
	//virtual void RotateX(float Angle) override;
	//virtual void RotateY(float Angle) override { Super::RotateY(Angle); bDirty = true; }
	//virtual void RotateZ(float Angle) override { Super::RotateZ(Angle); bDirty = true; }
};