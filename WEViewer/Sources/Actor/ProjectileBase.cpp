#include "..\..\..\WE\Sources\GameFramework\Object\Actor\ProjectileBase.h"
#include "ProjectileBase.h"
#include "World/World.h"
#include "Interface/HitInterface.h"
#include "Asset/BlueprintAsset.h"
#include "Component/BoxComponent.h"
#include "Component/SphereComponent.h"
#include "Component/ProjectileMovementComponent.h"
#include "Component/ObjectAnimComponent.h"
#include "Component/SplineCollisionComponent.h"
#include "Component/BoxCollisionComponent.h"
#include "Component/CapsuleCollisionComponent.h"
#include "Component/SphereCollisionComponent.h"
#include "Parser.h"

AProjectileBase::AProjectileBase()
{
	SetTickGroup(ETickGroup::ETG_PrePhysics, ETickPriority::ETP_High);

	RegisterWProperty("ImpactPoint", &mImpactPoint_Internal);
}

void AProjectileBase::Tick(float DeltaSecond)
{
	Super::Tick(DeltaSecond);
}

void AProjectileBase::BeginPlay()
{
	Super::BeginPlay();
}

void AProjectileBase::LoadWConfigs(const std::unordered_map<std::string, WAttributesMap>& Configs)
{
	Super::LoadWConfigs(Configs);
}

void DrawExplosion(XMFLOAT3 Location, float Radius, XMFLOAT4 Color, float Life)
{
	auto DrawLine = [&](float dx, float dy, float dz) {
		GetWorld()->DrawDebugLine(
			XMFLOAT3(Location.x - dx, Location.y - dy, Location.z - dz),
			XMFLOAT3(Location.x + dx, Location.y + dy, Location.z + dz),
			Color, Life
		);
	};

	// 기본 축 (XYZ)
	DrawLine(Radius, 0, 0); DrawLine(0, Radius, 0); DrawLine(0, 0, Radius);

	// 평면 대각선 (XY, YZ, XZ)
	float D = Radius * 0.7071f;
	DrawLine(D, D, 0); DrawLine(D, -D, 0);
	DrawLine(0, D, D); DrawLine(0, D, -D);
	DrawLine(D, 0, D); DrawLine(D, 0, -D);

	// 공간 대각선
	float D3 = Radius * 0.5773f;
	DrawLine(D3, D3, D3); DrawLine(D3, D3, -D3);
	DrawLine(D3, -D3, D3); DrawLine(D3, -D3, -D3);
}

void AProjectileBase::PlayParticle(XMFLOAT3 Loc, const std::string& Name)
{
	// 1. Red & Small (치명적인 불꽃 또는 작은 불꽃)
	if (Name == "P_Explosion_Red_Small")
	{
		float Radius = 2.5f;
		XMFLOAT4 Color = { 1.0f, 0.2f, 0.2f, 1.0f }; // 연한 빨강
		float Life = 0.3f; // 짧고 강렬하게
		DrawExplosion(Loc, Radius, Color, Life);
	}
	// 2. Red & Large (거대 화염 폭발)
	else if (Name == "P_Explosion_Red_Large")
	{
		float Radius = 12.0f;
		XMFLOAT4 Color = { 0.8f, 0.1f, 0.0f, 1.0f }; // 핏빛 빨강
		float Life = 0.8f;
		DrawExplosion(Loc, Radius, Color, Life);
	}
	// 3. Blue & Medium (차가운 마법 폭발)
	else if (Name == "P_Explosion_Blue")
	{
		float Radius = 6.0f;
		XMFLOAT4 Color = { 0.2f, 0.6f, 1.0f, 1.0f }; // 스카이 블루
		float Life = 0.5f;
		DrawExplosion(Loc, Radius, Color, Life);
	}
	// 4. Green & Tiny (독성 가스 분출)
	else if (Name == "P_Explosion_Green_Tiny")
	{
		float Radius = 1.5f;
		XMFLOAT4 Color = { 0.3f, 1.0f, 0.3f, 1.0f }; // 네온 그린
		float Life = 0.4f;
		DrawExplosion(Loc, Radius, Color, Life);
	}
	// 5. Purple & Huge (보스급 마법 또는 블랙홀 연출)
	else if (Name == "P_Explosion_Purple_Huge")
	{
		float Radius = 25.0f;
		XMFLOAT4 Color = { 0.5f, 0.0f, 0.8f, 1.0f }; // 진보라
		float Life = 1.2f; // 오래 남도록
		DrawExplosion(Loc, Radius, Color, Life);
	}
	// 6. Cyan & Spark (전기 스파크 연출)
	else if (Name == "P_Spark_Cyan")
	{
		float Radius = 3.0f;
		XMFLOAT4 Color = { 0.0f, 1.0f, 1.0f, 1.0f }; // 시안
		float Life = 0.2f; // 아주 빠르게 깜빡임
		DrawExplosion(Loc, Radius, Color, Life);
	}
	else
	{
		ShowMessageBox("Invalid particle name:\n" + Name);
		assert(false);
	}
}

void AProjectileBase::OnCollision(WSceneComponent* Instigator, WPhysicsComponent* HittedComp, XMFLOAT3 ImpactPoint, XMFLOAT3 Normal, float Distance, float Damage)
{
	mCommonOnHitEvent.Dispatch();
	TSharedPtr<AActor> Actor = HittedComp->GetOwner().lock();
	if (IHitInterface* HitInterface = dynamic_cast<IHitInterface*>(Actor.get()))
	{
		HitInterface->OnHit(Instigator, HittedComp, ImpactPoint, Normal, Damage);
	}
}