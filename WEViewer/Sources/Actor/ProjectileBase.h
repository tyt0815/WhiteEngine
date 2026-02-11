#pragma once
#include "Actor/Actor.h"

class AProjectileBase : public AActor
{
	typedef AActor Super;

public:
	AProjectileBase();

	virtual void Tick(float DeltaSecond) override;

	virtual void BeginPlay() override;

protected:
	virtual void LoadWConfigs(const std::unordered_map<std::string, WAttributesMap>& Configs) override;

	virtual void ApplyWComponentCommonAttribute(struct FBlueprintComponentNode* CompNode, WSceneComponent* Comp) override;

protected:
	void PlayParticle(XMFLOAT3 Loc, const std::string& Name);

private:
	void OnCollision(WSceneComponent* Instigator, WPhysicsComponent* HittedComp, XMFLOAT3 ImpactPoint, XMFLOAT3 Normal, float Distance, float Damage);

	TArray<WEvent> mOnHitEvents;
	WEvent mCommonOnHitEvent;

	XMFLOAT3 mImpactPoint_Internal;

	TArray<TSharedPtr<WEvent>> mOnLockonEvents;
	TArray<TSharedPtr<WEvent>> mOnBounceEvents;
	TArray<TSharedPtr<WEvent>> mOnAnimStopEvents;
};

REGISTER_ACTOR(AProjectileBase);