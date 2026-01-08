#pragma once
#include "SceneComponent.h"

class WLightComponent : public WSceneComponent
{
	typedef WSceneComponent Super;
public:
	WLightComponent();

protected:
	virtual void Update() override;

	DirectX::XMFLOAT3 mColor = DirectX::XMFLOAT3(1.0f, 1.0f, 1.0f);
	bool mbCastShadow = true;

public:
	inline void SetColor(DirectX::XMFLOAT3 Color)
	{
		mColor = Color;
	}
	inline void SetCastShadow(bool bCast)
	{
		mbCastShadow = bCast;
	}
};