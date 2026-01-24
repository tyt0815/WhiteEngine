#pragma once
#include "SceneComponent.h"

class WCameraComponent : public WSceneComponent
{
	typedef WSceneComponent Super;
public:
	WCameraComponent();
	virtual void SetOwner(TWeakPtr<AActor> Owner) override;

protected:
	virtual void Update() override;

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
	float mNearZ = 1;
	float mFarZ = 1000;
	float mAspectRatio = 1;
	bool mbViewFloat4x4Dirty = true;
	bool mbProjFloat4x4Dirty = true;
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
};