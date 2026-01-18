#pragma once
#include "World/DefaultWorld.h"
#include "Actor/Actor.h"
#include "Render/StaticMesh.h"

class WStaticMeshComponent;
class WProjectileMovementComponent;
class WSplineComponent;
class WBoxComponent;

class AHitReactor : public AActor
{
	typedef AActor Super;
public:
	AHitReactor();

	virtual void BeginPlay() override;

public:
	void AddTeleportTargets(const std::vector<TWeakPtr<WSplineComponent>>& NewTargets);

	void RandomTeleport();

private:
	TWeakPtr<WBoxComponent> mHitBoxComp;
	TWeakPtr<WStaticMeshComponent> mSMComp;

	std::vector<TWeakPtr<WSplineComponent>> mTeleportTargets;
};

class AProjectileActor : public AActor
{
public:
	AProjectileActor();

	XMFLOAT3 GetVelocity();

protected:
	TWeakPtr<WProjectileMovementComponent> mProjectileMovementComponent;
	
};

class ABullet : public AProjectileActor
{
	typedef AProjectileActor Super;
public:
	ABullet();

	virtual void BeginPlay() override;

protected:

private:
	TWeakPtr<WBoxComponent> mHitBoxComp;
	TWeakPtr<WStaticMeshComponent> mStaticMesh;

	void OnOverlap(TWeakPtr<WPhysicsComponent> Other, XMFLOAT3 ImpactPoint);
};

class ARingProjectile : public AProjectileActor
{
	typedef AProjectileActor Super;
public:
	ARingProjectile();

	virtual void BeginPlay() override;

	virtual void Tick_PostPhysics(float Delta) override;

private:
	void OnOverlapEvent(TWeakPtr<WPhysicsComponent> Other, XMFLOAT3 ImpactPoint);

	TWeakPtr<WStaticMeshComponent> mStaticMesh;

	std::vector<TWeakPtr<WBoxComponent>> mHitBoxes;

	std::vector<TWeakPtr<AActor>> ActorsToIgnore;
};

class ASplineBulletSpawner : public AActor
{
protected:
	class FSplineBullet
	{
	public:
		TWeakPtr<ABullet> SpawnBullet(WWorld* World);

		void RemoveAt(UINT i);

		TWeakPtr<WSplineComponent> Spline;
		TArray<TWeakPtr<ABullet>> Bullets;
		TArray<float> BulletDistance;
	};
};

class ASpiralBulletSpawner : public ASplineBulletSpawner
{
	typedef ASplineBulletSpawner Super;
public:
	ASpiralBulletSpawner();

	virtual void Tick_PostPhysics(float Delta) override;

private:
	FSplineBullet mSplineBullet;

	float mCoolDown = 0;
	float mCoolTime = 1;
};

class AWaveBulletSpanwer : public ASplineBulletSpawner
{
	typedef ASplineBulletSpawner Super;
public:
	AWaveBulletSpanwer();

	virtual void Tick_PostPhysics(float Delta) override;

public:
	std::vector<TWeakPtr<WSplineComponent>> GetSplines() const;

private:
	void UpdateSplineBullet(FSplineBullet* Spline, float Delta);

	std::vector<FSplineBullet> mSplines;

	float mCoolDown = 0;
	float mCoolTime = 1;
};

class ARingProjectileSpawner : public AActor
{
	typedef AActor Super;
public:
	ARingProjectileSpawner();

	virtual void Tick_PostPhysics(float Delta) override;

private:
	TWeakPtr<WSplineComponent> mSpline;

	TArray<TWeakPtr<ARingProjectile>> mRings;
	TArray<float> mDists;

	float mCoolDown = 0;
	float mCoolTime = 3;
};

class WSplineProjectileDemoWorld : public WDefaultWorld
{
public:
	WSplineProjectileDemoWorld();


};