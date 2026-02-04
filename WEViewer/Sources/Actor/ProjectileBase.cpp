#include "ProjectileBase.h"
#include "World/World.h"
#include "Interface/HitInterface.h"

AProjectileBase::AProjectileBase()
{
	if (auto Spline = CreateComponent<WSplineComponent>().lock())
	{
		Spline->SetupAttachment(GetRootComponent());
		Spline->LoadSplineFromAsset("RingSpline");
	}

	REGISTER_WFUNC_2(SetSmartHoming, SmartHoming, bool, Range, float);

	REGISTER_WFUNC_1(PlayParticle, ParticleName, std::string);

	REGISTER_WFUNC_1(SetTrailParticle, SceneComp, WSceneComponent*);

	REGISTER_WFUNC_1(MakeLineCollision, SceneComp, WSceneComponent*);

	REGISTER_WFUNC_RET_1(SelectRandomString, Strings, TArray<std::string>, std::string);

	mOnHit = RegisterWEvent("OnHit");

	SetTickGroup(ETickGroup::ETG_PrePhysics, ETickPriority::ETP_High);
}

void AProjectileBase::Tick(float DeltaSecond)
{
	Super::Tick(DeltaSecond);

	if (mbSmartHoming)
	{
		if (auto ProjComp = mProjMoveComp.lock())
		{
			if (!ProjComp->GetHomingTarget())
			{
				ProjComp->SetHoming(true);

				// Find Homing Target
				TArray<AActor*> ActorsToIgnore;
				ActorsToIgnore.push_back(this);
				TArray<FHitResult> Hits;
				GetWorld()->SphereOverlap(GetActorLocation(), mSmartHomingRange, ActorsToIgnore, Hits, false);
				for (const auto& Hit : Hits)
				{
					const auto& Actor = Hit.Actor.lock();
					if (IHitInterface* HitInter = dynamic_cast<IHitInterface*>(Actor.get()))
					{
						ProjComp->SetHomingTarget(Actor->GetRootComponent());
					}
				}
			}
		}
	}

	// Trail 그리기
	for (const auto& Info : mTrailComp)
	{
		XMFLOAT3 CurrLoc = Info->Target->GetWorldLocation();
		GetWorld()->DrawDebugLine(CurrLoc, Info->LastTickLocation, XMFLOAT4(1, 0, 0, 1), 0.25f);
	}

	// Line Collision 체크
	for (const auto& Info : mCollisionInfo)
	{
		XMFLOAT3 CurrLoc = Info.TargetInfo->Target->GetWorldLocation();
		const XMFLOAT3& LastLoc = Info.TargetInfo->LastTickLocation;
		TArray<AActor*> ActorsToIgnore;
		FHitResult Hit;
		GetWorld()->LineTrace(LastLoc, CurrLoc, ActorsToIgnore, Hit, true, 0.25f);
		if (TSharedPtr<AActor> Actor = Hit.Actor.lock())
		{
			OnHit();
		}
	}




	// 마지막에 실행
	for (auto& TrackedComp : mTrackedComp)
	{
		TrackedComp->LastTickLocation = TrackedComp->Target->GetWorldLocation();
	}
}

void AProjectileBase::BeginPlay()
{
	Super::BeginPlay();

	if (WProjectileMovementComponent* Comp = GetComponent<WProjectileMovementComponent>())
	{
		mProjMoveComp = Comp->GetWeakPtr<WProjectileMovementComponent>();
	}
}

void AProjectileBase::SetSmartHoming(bool bSmartHoming, float Range)
{
	TSharedPtr<WProjectileMovementComponent> ProjMoveComp = mProjMoveComp.lock();
	if (!ProjMoveComp)
	{
		if (WProjectileMovementComponent* Comp = GetComponent<WProjectileMovementComponent>())
		{
			mProjMoveComp = Comp->GetWeakPtr<WProjectileMovementComponent>();
			ProjMoveComp = mProjMoveComp.lock();
		}
		else
		{
			mbSmartHoming = false;
			return;
		}
	}

	mbSmartHoming = bSmartHoming;
	mSmartHomingRange = Range;
}

void AProjectileBase::OnHit()
{
	mOnHit->Dispatch();
}

void AProjectileBase::PlayParticle(const std::string& Name)
{
	if (Name == "Explosion")
	{
		XMFLOAT3 Location = GetActorLocation();
		float Radius = 5;
		XMFLOAT4 Color = { 1, 1, 0, 1 };
		float Life = 3;

		auto DrawLine = [&](float dx, float dy, float dz) {
			GetWorld()->DrawDebugLine(
				XMFLOAT3(Location.x - dx, Location.y - dy, Location.z - dz),
				XMFLOAT3(Location.x + dx, Location.y + dy, Location.z + dz),
				Color, Life
			);
		};

		// --- 기본축 ---
		DrawLine(Radius, 0, 0); // X
		DrawLine(0, Radius, 0); // Y
		DrawLine(0, 0, Radius); // Z

		// --- 평면 대각선 (45도 방향들) ---
		float D = Radius * 0.7071f; // sin(45) = 0.707
		DrawLine(D, D, 0);  // XY 대각선 1
		DrawLine(D, -D, 0); // XY 대각선 2
		DrawLine(0, D, D);  // YZ 대각선 1
		DrawLine(0, D, -D); // YZ 대각선 2
		DrawLine(D, 0, D);  // XZ 대각선 1
		DrawLine(D, 0, -D); // XZ 대각선 2

		// --- 완전 대각선 (정육면체 모서리 방향) ---
		float D3 = Radius * 0.5773f; // 1/sqrt(3)
		DrawLine(D3, D3, D3);
		DrawLine(D3, D3, -D3);
		DrawLine(D3, -D3, D3);
		DrawLine(D3, -D3, -D3);
	}


}

AProjectileBase::FTrackedSceneCompInfo* AProjectileBase::AddTrackedComp(WSceneComponent* Comp)
{
	// 1. 기존 요소 탐색
	auto Iter = std::find_if(mTrackedComp.begin(), mTrackedComp.end(),
		[Comp](const TUniquePtr<FTrackedSceneCompInfo>& Info)
		{
			return Info->Target == Comp;
		}
	);

	// 2. 이미 존재하면 즉시 리턴
	if (Iter != mTrackedComp.end())
	{
		return Iter->get();
	}

	// 3. 없으면 새로 생성 및 추가
	auto Info = MakeUnique<FTrackedSceneCompInfo>();
	Info->Target = Comp;
	Info->LastTickLocation = Comp->GetWorldLocation();

	// 포인터를 꺼내놓고 push_back 합니다. (std::move 이후에는 Info를 쓸 수 없으므로)
	FTrackedSceneCompInfo* RawPtr = Info.get();
	mTrackedComp.push_back(std::move(Info));

	return RawPtr;
}

void AProjectileBase::SetTrailParticle(WSceneComponent* Comp)
{
	FTrackedSceneCompInfo* Info = AddTrackedComp(Comp);

	if (std::count(mTrailComp.begin(), mTrailComp.end(), Info) == 0)
	{
		mTrailComp.push_back(Info);
	}
}

void AProjectileBase::MakeLineCollision(WSceneComponent* Comp)
{
	FTrackedSceneCompInfo* Info = AddTrackedComp(Comp);
	const auto Iter = std::find_if(
		mCollisionInfo.begin(), mCollisionInfo.end(),
		[Info](const FMakeCollisionInfo& A) 
		{ 
			return A.TargetInfo == Info; 
		}
	);
	if (Iter == mCollisionInfo.end())
	{
		FMakeCollisionInfo CollInfo;
		CollInfo.TargetInfo = Info;
		CollInfo.Type = FMakeCollisionInfo::EType::ET_Line;
		mCollisionInfo.push_back(std::move(CollInfo));
	}
}

std::string AProjectileBase::SelectRandomString(const TArray<std::string>& Strings)
{
	return Strings[FDXMath::Rand(0, (int)Strings.size() - 1)];
}