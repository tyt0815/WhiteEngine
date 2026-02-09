#pragma once
#include "Actor/Actor.h"
#include "Physics/HitResult.h"
#include "Component/BoxComponent.h"
#include "Component/ProjectileMovementComponent.h"
#include "Component/ObjectAnimComponent.h"
#include "Component/SplineComponent.h"

class AProjectileBase : public AActor
{
	typedef AActor Super;

public:
	AProjectileBase();

	virtual void Tick(float DeltaSecond) override;

	virtual void BeginPlay() override;

protected:
	virtual void LoadWConfigs(const std::unordered_map<std::string, WAttributesMap>& Configs) override;

	virtual void OnLoadWComponent(struct FBlueprintComponentNode* CompNode, WSceneComponent* Comp) override;

protected:

	void SetSmartHoming(bool bSmartHoming, float Range);

	void PlayParticle(const std::string& Name);

private:
	void OnCollision(AActor* Actor, WPhysicsComponent* Comp, XMFLOAT3 ImpactPoint, XMFLOAT3 Normal, float Distance);

	TWeakPtr<WProjectileMovementComponent> mProjMoveComp;
	TWeakPtr<WObjectAnimComponent> mObjAnimComp;

	std::unordered_map<std::string, class WObjectAnimComponent*> mWObjAnimComp;

	const WEvent* mOnHitEvent;

	float mMaxSpeed = 1;

	float mSmartHomingRange = 0.0f;

	float mDamage = 1.0f;

	bool mbSmartHoming = false;
};

REGISTER_ACTOR(AProjectileBase);