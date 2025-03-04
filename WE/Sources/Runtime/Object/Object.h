#pragma once

#include "DirectX/DXMath.h"

class WObject
{
public:
	WObject() = default;
	virtual void Tick(float Delta) {};

	inline XMMATRIX GetWorldMatrix() { return Transform.GetTransformMatrix(); }
	inline FTransform GetTransform() const { return Transform; }
	inline XMFLOAT3 GetScale() const { return Transform.Scale; }
	inline XMFLOAT3 GetRotation() const { return Transform.Rotation; }
	inline XMFLOAT3 GetTranslation() const { return Transform.Translation; }
	inline void SetTransform(FTransform NewTransform) { Transform = NewTransform; }
	inline virtual void SetTranslation(XMFLOAT3 Translation) { Transform.Translation = Translation; }
	inline void SetRotation(XMFLOAT3 Rotation) { Transform.Rotation = Rotation; }
	inline void SetScale(XMFLOAT3 Scale) { Transform.Scale = Scale; }

	virtual void RotateX(float Angle) { Transform.Rotation.x += Angle; }
	virtual void RotateY(float Angle) { Transform.Rotation.y += Angle; }
	virtual void RotateZ(float Angle) { Transform.Rotation.z += Angle; }

protected:
	FTransform Transform = FTransform::Default;
};