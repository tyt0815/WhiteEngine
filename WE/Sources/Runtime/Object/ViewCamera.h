#pragma once

#include "Object.h"

class WViewCamera : public WObject
{
	using Super = WObject;
public:
	WViewCamera();

	inline XMMATRIX GetViewMatrix() const { return XMLoadFloat4x4(&View); }
	inline XMMATRIX GetProjMatrix() const { return XMLoadFloat4x4(&Proj); }
	virtual void SetTranslation(XMFLOAT3 Translation) override { Super::SetTranslation(Translation); bDirty = true; }
	virtual void RotateX(float Angle) override;
	virtual void RotateY(float Angle) override { Super::RotateY(Angle); bDirty = true; }
	virtual void RotateZ(float Angle) override { Super::RotateZ(Angle); bDirty = true; }

	void UpdateViewMatrix();
	void UpdateProjMatrix(float FovY, float AspectRatio, float NearZ, float FarZ);
	void Move(float X, float Y, float Z);

private:
	XMFLOAT4X4 View = FDXMath::Identity4x4();
	XMFLOAT4X4 Proj = FDXMath::Identity4x4();
	XMFLOAT3 Right, Up, Look;
	bool bDirty = true;
};