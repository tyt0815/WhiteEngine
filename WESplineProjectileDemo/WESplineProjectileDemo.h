#pragma once
#include "GameFramework/Object/World/DefaultWorld.h"
#include "GameFramework/Object/Actor/Actor.h"
#include "Render/StaticMesh.h"

class WStaticMeshComponent;
class WProjectileMovementComponent;
class WSplineComponent;

class ABullet : public AActor
{
public:
	ABullet();

public:
	XMFLOAT3 GetVelocity();

protected:

private:
	WStaticMeshComponent* mStaticMesh;
	WProjectileMovementComponent* mProjectileMovementComponent;
};

class ABulletSpawner : public AActor
{

};

class ASpiralBulletSpawner : public ABulletSpawner
{
public:
	ASpiralBulletSpawner();

	virtual void Tick_PostPhysics(float Delta) override;

private:
	WSplineComponent* mSpiralSpline;

	TUnorderedArray<ABullet*> mBullets;
	TUnorderedArray<float> mBulletDistance;

	float mCoolDown = 0;
	float mCoolTime = 1;
};

class WSplineProjectileDemoWorld : public WDefaultWorld
{
public:
	WSplineProjectileDemoWorld();


};