#pragma once

#include "DirectX/DXUtility.h"

class WObject
{
public:
	WObject() = default;
	inline XMMATRIX GetWorldMatrix() { return Transform.GetTransformMatrix(); }
	inline void SetTransform(FTransform NewTransform) { Transform = NewTransform; }
	inline void SetTranslation(XMFLOAT3 Translation) { Transform.Translation = Translation; }
	inline void SetRotation(XMFLOAT3 Rotation) { Transform.Rotation = Rotation; }
	inline void SetScale(XMFLOAT3 Scale) { Transform.Scale = Scale; }

protected:
	FTransform Transform = FTransform::Default;
};