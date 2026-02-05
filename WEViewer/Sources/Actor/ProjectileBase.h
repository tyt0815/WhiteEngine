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
	struct FTrackedSceneCompInfo
	{
		WSceneComponent* Target;
		XMFLOAT3 LastTickLocation;
	};

	struct FMakeCollisionInfo
	{
		enum class EType
		{
			ET_Line,
			ET_Box,
		};

		FTrackedSceneCompInfo* TargetInfo;
		EType	 Type;
		union
		{
			XMFLOAT3 Extent;
		};
	};

public:
	AProjectileBase();

	virtual void Tick(float DeltaSecond) override;

	virtual void BeginPlay() override;

protected:
	virtual void OnHit(TWeakPtr<WPhysicsComponent> Comp, XMFLOAT3 ImpactPoint);

	void SetSmartHoming(bool bSmartHoming, float Range);

	void PlayParticle(const std::string& Name);

	FTrackedSceneCompInfo* AddTrackedComp(WSceneComponent* Comp);

	void SetTrailParticle(WSceneComponent* Comp);

	void MakeLineCollision(WSceneComponent* Comp);
	
	std::string SelectRandomString(const TArray<std::string>& Strings);

	void CreateBoxTraceHitBySplineComponent(WSplineComponent* SplineComponent, int Segment);

	void CreateBoxColliderBySplineComponent(WSplineComponent* SplineComponent, int Segment);

private:
	TWeakPtr<WProjectileMovementComponent> mProjMoveComp;

	bool mbSmartHoming = false;

	float mSmartHomingRange = 0.0f;

	TArray<TUniquePtr<FTrackedSceneCompInfo>> mTrackedComp;

	TArray<FTrackedSceneCompInfo*> mTrailComp;

	TArray<FMakeCollisionInfo> mCollisionInfo;

	TArray<FTrackedSceneCompInfo*> mBoxCollisionInfo;

	const WEvent* mOnHit;
};

REGISTER_ACTOR(AProjectileBase);