#pragma once
#include "Actor/Actor.h"
#include "Component/BoxComponent.h"
#include "Component/ProjectileMovementComponent.h"
#include "Component/StaticMeshComponent.h"
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
	virtual void LoadBlueprintAttribute(const FBlueprintAttributesMap& Attributes) override;

	void SetSmartHoming(bool bSmartHoming, float Range);

	void PlayParticle(const std::string& Name);

private:
	TWeakPtr<WProjectileMovementComponent> mProjMoveComp;

	bool mbSmartHoming = false;

	float mSmartHomingRange = 0.0f;
};

REGISTER_ACTOR(AProjectileBase);