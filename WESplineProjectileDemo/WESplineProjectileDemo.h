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
	void AddTeleportTargets(const std::vector<WSplineComponent*>& NewTargets);

	void RandomTeleport();

private:
	WBoxComponent* mHitBoxComp;
	WStaticMeshComponent* mSMComp;

	void OnOverlapEvent(WPrimitiveComponent* Other);

	std::vector<WSplineComponent*> mTeleportTargets;
};

class AProjectileActor : public AActor
{
public:
	AProjectileActor();

	XMFLOAT3 GetVelocity();

protected:
	WProjectileMovementComponent* mProjectileMovementComponent;
	
};

class ABullet : public AProjectileActor
{
	typedef AProjectileActor Super;
public:
	ABullet();

	virtual void BeginPlay() override;

protected:

private:
	WBoxComponent* mHitBoxComp;
	WStaticMeshComponent* mStaticMesh;

	void OnOverlap(WPrimitiveComponent* Other);
};

class ARingProjectile : public AProjectileActor
{
public:
	ARingProjectile();

private:
	WStaticMeshComponent* mStaticMesh;
};

class ASplineBulletSpawner : public AActor
{
protected:
	class FSplineBullet
	{
	public:
		ABullet* SpawnBullet(WWorld* World);

		void RemoveAt(UINT i);

		WSplineComponent* Spline;
		TUnorderedArray<ABullet*> Bullets;
		TUnorderedArray<float> BulletDistance;
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
	std::vector<WSplineComponent*> GetSplines() const;

private:
	void UpdateSplineBullet(FSplineBullet* Spline, float Delta);

	std::vector<FSplineBullet> mSplines;

	float mCoolDown = 0;
	float mCoolTime = 1;
};

class WSplineProjectileDemoWorld : public WDefaultWorld
{
public:
	WSplineProjectileDemoWorld();


};