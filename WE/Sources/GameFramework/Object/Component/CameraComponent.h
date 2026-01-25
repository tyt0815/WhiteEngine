#pragma once
#include "SceneComponent.h"

class WCameraComponent : public WSceneComponent
{
	typedef WSceneComponent Super;
public:
	WCameraComponent();
	virtual void SetOwner(TWeakPtr<AActor> Owner) override;

public:

	DirectX::XMFLOAT4X4 GetViewMatrix();

	DirectX::XMFLOAT4X4 GetProjMatrix();

protected:

	virtual void OnSetTransform() override;

private:
	void UpdateViewMatrix();
	void UpdateProjMatrix();
	XMFLOAT4X4 mView = FDXMath::Identity4x4();
	XMFLOAT4X4 mProj = FDXMath::Identity4x4();
	XMFLOAT3 mRight = {};
	XMFLOAT3 mUp = {};
	XMFLOAT3 mLook = {};
	float mFov = 90;
	float mNearZ = 0.1f;
	float mFarZ = 1000.0f;
	float mAspectRatio = 1;
	bool mbViewFloat4x4Dirty = true;
	bool mbProjFloat4x4Dirty = true;
public:
	inline float GetNearZ() const
	{
		return mNearZ;
	}
	inline float GetFarZ() const
	{
		return mFarZ;
	}
};