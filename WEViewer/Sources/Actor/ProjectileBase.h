#pragma once
#include "Actor/Actor.h"
#include "Component/BoxComponent.h"
#include "Component/ProjectileMovementComponent.h"
#include "Component/ObjectAnimComponent.h"
#include "Component/SplineComponent.h"

class AProjectileBase : public AActor
{
	typedef AActor Super;
	//struct FTrackedSceneCompInfo
	//{
	//	WSceneComponent* Target;
	//	XMFLOAT3 LastTickLocation;
	//};

	//struct FMakeCollisionInfo
	//{
	//	enum class EType
	//	{
	//		ET_Line,
	//		ET_Box,
	//	};

	//	FTrackedSceneCompInfo* TargetInfo;
	//	EType	 Type;
	//	union
	//	{
	//		XMFLOAT3 Extent;
	//	};
	//};

public:
	AProjectileBase();

	virtual void Tick(float DeltaSecond) override;

	virtual void BeginPlay() override;

protected:
	virtual void LoadWAttributes(const WAttributesMap& Attributes) override;

	virtual void OnLoadWComponent(struct FBlueprintComponentNode* CompNode, WSceneComponent* Comp) override;

protected:

	void SetSmartHoming(bool bSmartHoming, float Range);

	void PlayParticle(const std::string& Name);

private:
	TWeakPtr<WProjectileMovementComponent> mProjMoveComp;
	TWeakPtr<WObjectAnimComponent> mObjAnimComp;

	std::unordered_map<std::string, class WObjectAnimComponent*> mWObjAnimComp;

	float mMaxSpeed = 1;

	float mSmartHomingRange = 0.0f;

	bool mbSmartHoming = false;
};

REGISTER_ACTOR(AProjectileBase);