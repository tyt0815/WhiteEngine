#pragma once

#include "DirectX/DXMath.h"

class WObject
{
public:
	WObject() = default;
	XMMATRIX GetWorldMatrix();
	inline void SetTransform(FTransform NewTransform) { Transform = NewTransform; }
	inline void SetTranslation(XMFLOAT3 Translation) { Transform.Translation = Translation; }
	inline void SetRotation(XMFLOAT3 Rotation) { Transform.Rotation = Rotation; }
	inline void SetScale(XMFLOAT3 Scale) { Transform.Scale = Scale; }

protected:
	FTransform Transform;
};