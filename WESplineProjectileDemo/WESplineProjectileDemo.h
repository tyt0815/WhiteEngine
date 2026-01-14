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