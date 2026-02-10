#include "ProjectileBase.h"
#include "World/World.h"
#include "Interface/HitInterface.h"
#include "Asset/BlueprintAsset.h"
#include "Component/BoxComponent.h"
#include "Component/SphereComponent.h"
#include "Component/SplineCollisionComponent.h"
#include "Component/BoxCollisionComponent.h"
#include "Component/CapsuleCollisionComponent.h"
#include "Component/SphereCollisionComponent.h"

AProjectileBase::AProjectileBase()
{
	SetTickGroup(ETickGroup::ETG_PrePhysics, ETickPriority::ETP_High);

	mProjMoveComp = CreateComponent<WProjectileMovementComponent>()->GetWeakPtr<WProjectileMovementComponent>();
	auto ProjComp = mProjMoveComp.lock();
	mObjAnimComp = CreateComponent<WObjectAnimComponent>()->GetWeakPtr<WObjectAnimComponent>();
	auto AnimComp = mObjAnimComp.lock();

	////////////////////////////////////////////////////////////////////////////////////////////////
	// 
	// WComponent
	// 
	////////////////////////////////////////////////////////////////////////////////////////////////

	RegisterWComponentFactory("Collision", [this](const WAttributesMap& Attributes)
		{
			WSceneComponent* Component = nullptr;
			const std::string& Name = Attributes.at("Name");
			const std::string& Type = Attributes.at("Type");
			if (Type == "Spline")
			{
				WSplineCollisionComponent* Comp = this->CreateComponent<WSplineCollisionComponent>();

				ApplyAttribute<std::string>(Attributes, "Asset", [&](const std::string& v) {
					Comp->LoadSplineFromAsset(v);
					});

				int Segment = 1;
				ExtractAttribute(Attributes, "Segment", Segment);
				Comp->SetSegment(Segment);

				bool bUseBoundingBox = true;
				ExtractAttribute(Attributes, "BoundingBox", bUseBoundingBox);
				Comp->SetBoundingBox(bUseBoundingBox);

				Component = Comp;
			}
			else if (Type == "Box")
			{
				WBoxCollisionComponent* Comp = this->CreateComponent<WBoxCollisionComponent>();
				
				ApplyAttribute<XMFLOAT3>(Attributes, "Extent", [&](const XMFLOAT3& v) {
					Comp->SetExtent(v);
					});

				Component = Comp;
			}
			else if (Type == "Capsule")
			{
				WCapsuleCollisionComponent* Comp = this->CreateComponent<WCapsuleCollisionComponent>();

				// 반지름(Radius) 설정
				ApplyAttribute<float>(Attributes, "Radius", [&](float v) {
					float HalfHeight = 0.0f; // 기존 값을 유지하기 위해 임시 저장소 필요 시 로직 조정
					Comp->SetCapsuleSize(v, 1.0f); // 기본 HalfHeight 예시
					});

				// 반높이(HalfHeight) 설정
				ApplyAttribute<float>(Attributes, "HalfHeight", [&](float v) {
					Comp->SetCapsuleSize(1.0f, v);
					});

				Component = Comp;
			}
			else if (Type == "Sphere")
			{
				WSphereCollisionComponent* Comp = this->CreateComponent<WSphereCollisionComponent>();

				// 반지름(Radius) 설정
				ApplyAttribute<float>(Attributes, "Radius", [&](float v) {
					Comp->SetRadius(v);
					});

				Component = Comp;
			}
			else
			{
				ShowMessageBox("Invalid collision type:\n" + Type);
				assert(false);
			}


			if (FCollisionGeneratorBase* CollisionGenerator = dynamic_cast<FCollisionGeneratorBase*>(Component))
			{
				CollisionGenerator->GenerateCollision();
				CollisionGenerator->mOnCollision.Add(this, &AProjectileBase::OnCollision);

				ApplyAttribute<float>(Attributes, "Delay", [=](auto&& v)
					{
						CollisionGenerator->SetHitDelay(v);
					});
				RegisterWProperty(Name + ".Delay", &CollisionGenerator->mHitDelay);

				ApplyAttribute<int>(Attributes, "MaxHit", [=](auto&& v)
					{
						CollisionGenerator->SetMaxHit(v);
					});
				RegisterWProperty(Name + ".MaxHit", &CollisionGenerator->mMaxHit);

				ApplyAttribute<float>(Attributes, "Damage", [=](auto&& v)
					{
						CollisionGenerator->SetDamage(v);
					});
				RegisterWProperty(Name + ".Damage", &CollisionGenerator->mDamage);

				ApplyAttribute<std::set<std::string>>(Attributes, "TargetTags", [=](auto&& Arry)
					{
						CollisionGenerator->AddTargetTags(Arry);
					});
				RegisterWProperty(Name + ".TargetTags", &CollisionGenerator->mTargetTags);

				ApplyAttribute<bool>(Attributes, "Debug", [=](auto&& v) 
					{
						CollisionGenerator->SetDebug(v);
					});
				RegisterWProperty(Name + ".Debug", &CollisionGenerator->mbDebug);
			}


			return Component;
		});

	////////////////////////////////////////////////////////////////////////////////////////////////
	// 
	// WComponent End
	// 
	////////////////////////////////////////////////////////////////////////////////////////////////

	////////////////////////////////////////////////////////////////////////////////////////////////
	// 
	// WAction
	// 
	////////////////////////////////////////////////////////////////////////////////////////////////

	RegisterWActionFactory("Particle", [this](const WAttributesMap& Attributes) {
		std::string Name = Attributes.at("Asset");
		return [this, Name]() { this->PlayParticle(Name); };
		});

	RegisterWActionFactory("Animation", [this, AnimComp](const WAttributesMap& Attributes) {
		WObjectAnimComponent* Anim = nullptr;
		if (Attributes.count("Target"))
		{
			assert(mWObjAnimComp.count(Attributes.at("Target")) > 0 && "Set Component Anim = true");
			Anim = mWObjAnimComp.at(Attributes.at("Target"));
		}
		else
		{
			Anim = AnimComp.get();
		}

		float PlayRate = 1;
		ExtractAttribute(Attributes, "PlayRate", PlayRate);
		bool bLoop = false;
		ExtractAttribute(Attributes, "Loop", bLoop);

		uint16_t Flags = 0;
		std::string LocFlag;
		ExtractAttribute(Attributes, "Loc", LocFlag);
		if (LocFlag.find('X') != std::string::npos)
		{
			Flags |= ERootMotion::LocX;
		}
		if (LocFlag.find('Y') != std::string::npos)
		{
			Flags |= ERootMotion::LocY;
		}
		if (LocFlag.find('Z') != std::string::npos)
		{
			Flags |= ERootMotion::LocZ;
		}
		std::string RotFlag;
		ExtractAttribute(Attributes, "Rot", RotFlag);
		if (RotFlag.find('X') != std::string::npos)
		{
			Flags |= ERootMotion::RotX;
		}
		if (RotFlag.find('Y') != std::string::npos)
		{
			Flags |= ERootMotion::RotY;
		}
		if (RotFlag.find('Z') != std::string::npos)
		{
			Flags |= ERootMotion::RotZ;
		}
		std::string ScaleFlag;
		ExtractAttribute(Attributes, "Scale", ScaleFlag);
		if (ScaleFlag.find('X') != std::string::npos)
		{
			Flags |= ERootMotion::ScaleX;
		}
		if (ScaleFlag.find('Y') != std::string::npos)
		{
			Flags |= ERootMotion::ScaleY;
		}
		if (ScaleFlag.find('Z') != std::string::npos)
		{
			Flags |= ERootMotion::ScaleZ;
		}

		if (Flags == 0)
		{
			Flags = ERootMotion::All;
		}

		WAction Action;
		if (Attributes.count("Asset") > 0)
		{
			std::string AssetName = Attributes.at("Asset");
			std::string AnimName = Attributes.at("Anim");
			Action = [=]() { Anim->LoadAndPlay(AssetName, AnimName, PlayRate, bLoop, Flags); };
		}
		else
		{
			Action = [=]() { Anim->Play(PlayRate, bLoop, Flags); };
		}

		return Action;
		});

	RegisterWActionFactory("CurveBind", [this](const WAttributesMap& Attributes)
		{
			std::string TargetName = Attributes.at("Target");
			float* Prop = std::get<float*>(GetWProperty(TargetName));

			WObjectAnimComponent* AnimComp = Attributes.count("Comp") > 0 ? mWObjAnimComp[Attributes.at("Comp")] : mObjAnimComp.lock().get();

			std::string CurveName = Attributes.at("Curve");

			bool bIsModifier = false;
			ExtractAttribute(Attributes, "Modifier", bIsModifier);

			return [=]()
			{
				AnimComp->BindCurve(CurveName, Prop, bIsModifier, *Prop);
			};
		});

	////////////////////////////////////////////////////////////////////////////////////////////////
	// 
	// WAction End
	// 
	////////////////////////////////////////////////////////////////////////////////////////////////

	////////////////////////////////////////////////////////////////////////////////////////////////
	// 
	// WProperty
	// 
	////////////////////////////////////////////////////////////////////////////////////////////////

	RegisterWProperty("Movement.MaxSpeed", &ProjComp->mMaxSpeed);
	RegisterWProperty("Movement.Acceleration", &ProjComp->mAcceleration);
	RegisterWProperty("Movement.GravityScale", &ProjComp->mGravityScale);

	RegisterWProperty("LifeCycle.LifeSpan", &ProjComp->mLifeSpan);


	RegisterWProperty("Homing.TargetTags", &mHomingTargetTags);
	RegisterWProperty("Homing.Range", &mHomingRange);
	RegisterWProperty("Homing.Angle", &mHomingAngle);
	RegisterWProperty("Homing.RetargetTick", &mRetargetTick);
	RegisterWProperty("Homing.TurnRate", &ProjComp->mHomingTurnLimit);
	RegisterWProperty("Homing.StopRange", &mHomingStopRange);
	RegisterWProperty("Homing.ForgetPrev", &mbForgetPreviousTarget);
	RegisterWProperty("Waypoint.Use", &mbUseWaypoints);
	RegisterWProperty("Waypoint.Space", &mWaypointSpace); 
	RegisterWProperty("Waypoint.Base", &mWaypointBase);   
	RegisterWProperty("Waypoint.Type", &mWaypointType);   

	float mHomingTurnLimit = 0;

	////////////////////////////////////////////////////////////////////////////////////////////////
	// WProperty End
	////////////////////////////////////////////////////////////////////////////////////////////////

	////////////////////////////////////////////////////////////////////////////////////////////////
	// WEvent
	////////////////////////////////////////////////////////////////////////////////////////////////

	RegisterSystemEvent("OnHit", [this](const WAttributesMap& Attributes) -> WEvent* {
		auto Iter = Attributes.find("Target");

		WEvent* Event = &mCommonOnHitEvent;
		if (Iter != Attributes.end())
		{
			const std::string& Target = Iter->second;
			mOnHitEvents.push_back({});
			Event = &mOnHitEvents.back();

			if (FCollisionGeneratorBase* CollisionGenerator = dynamic_cast<FCollisionGeneratorBase*>(GetWComponent(Target)))
			{
				CollisionGenerator->mOnCollision.AddLambda([Event](auto&&...)
					{
						Event->Dispatch();
					});
			}
			else
			{
				ShowMessageBox("OnHit: Invalid target\n" + Target);
			}
		}

		return Event;
		});

	////////////////////////////////////////////////////////////////////////////////////////////////
	// WEvent End
	////////////////////////////////////////////////////////////////////////////////////////////////
}

void AProjectileBase::Tick(float DeltaSecond)
{
	Super::Tick(DeltaSecond);

	UpdateHoming(DeltaSecond);
}

void AProjectileBase::BeginPlay()
{
	Super::BeginPlay();

	if (WProjectileMovementComponent* Comp = GetComponent<WProjectileMovementComponent>())
	{
		mProjMoveComp = Comp->GetWeakPtr<WProjectileMovementComponent>();
	}
}

void AProjectileBase::LoadWConfigs(const std::unordered_map<std::string, WAttributesMap>& Configs)
{
	Super::LoadWConfigs(Configs);

	TSharedPtr<WProjectileMovementComponent> ProjMoveComp = mProjMoveComp.lock();

	if (Configs.count("Movement"))
	{
		const WAttributesMap& Attributes = Configs.at("Movement");

		// 1. 초기 속도 (Initial Velocity) - XMFLOAT3
		ApplyAttribute<XMFLOAT3>(Attributes, "InitSpeed", [&](const XMFLOAT3& v) {
			ProjMoveComp->SetInitialVelocity(v);
			});

		// 2. 최대 속도 (Max Speed) - float
		ApplyAttribute<float>(Attributes, "MaxSpeed", [&](float v) {
			ProjMoveComp->SetMaxSpeed(v);
			});

		// 3. 가속도 (Acceleration) - float
		ApplyAttribute<float>(Attributes, "Acceleration", [&](float v) {
			ProjMoveComp->SetAcceleration(v);
			});

		// 4. 중력 배율 (Gravity Scale) - float
		ApplyAttribute<float>(Attributes, "GravityScale", [&](float v) {
			ProjMoveComp->SetGravityScale(v);
			});
	}

	if (Configs.count("LifeCycle"))
	{
		const WAttributesMap& Attributes = Configs.at("LifeCycle");

		// 5. 수명 (LifeSpan / Life) - float
		ApplyAttribute<float>(Attributes, "Life", [&](float v) {
			ProjMoveComp->SetLifeSpan(v);
			});
	}

	if (Configs.count("Homing"))
	{
		const WAttributesMap& Attributes = Configs.at("Homing");

		// 1. 기본 호밍 설정
		ApplyAttribute<std::string>(Attributes, "Strategy", [&](const std::string& v) {
			if (v == "Nearest") mHomingStrategy = EHomingStrategy::Nearest;
			else if (v == "Angle") mHomingStrategy = EHomingStrategy::Angle;
			else mHomingStrategy = EHomingStrategy::None;
			});

		ExtractAttribute(Attributes, "Range", mHomingRange);
		ExtractAttribute(Attributes, "Angle", mHomingAngle);
		ExtractAttribute(Attributes, "RetargetTick", mRetargetTick);
		ExtractAttribute(Attributes, "StopRange", mHomingStopRange);
		ExtractAttribute(Attributes, "ForgetPrev", mbForgetPreviousTarget);

		ApplyAttribute<float>(Attributes, "TurnRate", [&](float v) {
			ProjMoveComp->SetHomingTurnLimit(v);
			});

		ApplyAttribute<std::set<std::string>>(Attributes, "TargetTags", [&](const std::set<std::string>& v) {
			mHomingTargetTags = v;
			});

		// 2. Waypoint 설정
		ExtractAttribute(Attributes, "UseWaypoints", mbUseWaypoints);
		if (mbUseWaypoints)
		{
			ExtractAttribute(Attributes, "Space", mWaypointSpace); // "Direction" or "World"
			ExtractAttribute(Attributes, "Base", mWaypointBase);   // "Actor" or "Target"
			ExtractAttribute(Attributes, "Type", mWaypointType);   // "Value" or "Adaptive"

			// Waypoints 리스트 파싱 (XML에서 "Offsets" 같은 키로 float3 배열을 받는다고 가정)
			ApplyAttribute<std::vector<XMFLOAT3>>(Attributes, "Waypoints", [&](const std::vector<XMFLOAT3>& v) {
				mConfigWaypoints = v;
				});
		}
	}
}

void AProjectileBase::ApplyWComponentCommonAttribute(FBlueprintComponentNode* CompNode, WSceneComponent* Comp)
{
	Super::ApplyWComponentCommonAttribute(CompNode, Comp);

	ApplyAttribute<bool>(CompNode->Attributes, "Anim", [=](bool v) {
		if (v)
		{
			WObjectAnimComponent* AnimComp = CreateComponent<WObjectAnimComponent>();
			AnimComp->SetTargetComponent(Comp);
			mWObjAnimComp[CompNode->Attributes["Name"]] = AnimComp;
		}
		});
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

void AProjectileBase::PlayParticle(const std::string& Name)
{
	// 1. Red & Small (치명적인 불꽃 또는 작은 불꽃)
	if (Name == "P_Explosion_Red_Small")
	{
		float Radius = 2.5f;
		XMFLOAT4 Color = { 1.0f, 0.2f, 0.2f, 1.0f }; // 연한 빨강
		float Life = 0.3f; // 짧고 강렬하게
		DrawExplosion(GetActorLocation(), Radius, Color, Life);
	}
	// 2. Red & Large (거대 화염 폭발)
	else if (Name == "P_Explosion_Red_Large")
	{
		float Radius = 12.0f;
		XMFLOAT4 Color = { 0.8f, 0.1f, 0.0f, 1.0f }; // 핏빛 빨강
		float Life = 0.8f;
		DrawExplosion(GetActorLocation(), Radius, Color, Life);
	}
	// 3. Blue & Medium (차가운 마법 폭발)
	else if (Name == "P_Explosion_Blue")
	{
		float Radius = 6.0f;
		XMFLOAT4 Color = { 0.2f, 0.6f, 1.0f, 1.0f }; // 스카이 블루
		float Life = 0.5f;
		DrawExplosion(GetActorLocation(), Radius, Color, Life);
	}
	// 4. Green & Tiny (독성 가스 분출)
	else if (Name == "P_Explosion_Green_Tiny")
	{
		float Radius = 1.5f;
		XMFLOAT4 Color = { 0.3f, 1.0f, 0.3f, 1.0f }; // 네온 그린
		float Life = 0.4f;
		DrawExplosion(GetActorLocation(), Radius, Color, Life);
	}
	// 5. Purple & Huge (보스급 마법 또는 블랙홀 연출)
	else if (Name == "P_Explosion_Purple_Huge")
	{
		float Radius = 25.0f;
		XMFLOAT4 Color = { 0.5f, 0.0f, 0.8f, 1.0f }; // 진보라
		float Life = 1.2f; // 오래 남도록
		DrawExplosion(GetActorLocation(), Radius, Color, Life);
	}
	// 6. Cyan & Spark (전기 스파크 연출)
	else if (Name == "P_Spark_Cyan")
	{
		float Radius = 3.0f;
		XMFLOAT4 Color = { 0.0f, 1.0f, 1.0f, 1.0f }; // 시안
		float Life = 0.2f; // 아주 빠르게 깜빡임
		DrawExplosion(GetActorLocation(), Radius, Color, Life);
	}
	else
	{
		ShowMessageBox("Invalid particle name:\n" + Name);
		assert(false);
	}
}

void AProjectileBase::OnCollision(AActor* Actor, WPhysicsComponent* Comp, XMFLOAT3 ImpactPoint, XMFLOAT3 Normal, float Distance, float Damage)
{
	mCommonOnHitEvent.Dispatch();

	if (IHitInterface* HitInterface = dynamic_cast<IHitInterface*>(Actor))
	{
		HitInterface->OnHit(this, Comp, ImpactPoint, Normal, Damage);
	}
}

void AProjectileBase::SetHomingTarget(AActor* Target)
{
	if (mCurrentTarget == Target)
	{
		return;
	}

	mCurrentTarget = Target;

	if (mCurrentTarget == nullptr)
	{
		mProjMoveComp.lock()->SetHoming(false);
		return;
	}

	auto ProjComp = mProjMoveComp.lock();
	ProjComp->SetHoming(true);
	if (mbUseWaypoints)
	{
		GenerateWaypoints(Target);
	}

	if (mFinalWaypoints.size() > 1)
	{
		ProjComp->SetHomingLocation(mFinalWaypoints.front());
	}
	else
	{
		ProjComp->SetHomingTarget(mCurrentTarget->GetRootComponent());
	}

}

void AProjectileBase::UpdateHoming(float DeltaSecond)
{
	auto ProjComp = mProjMoveComp.lock();
	if (!ProjComp) return;

	// 1. 현재 경유지/타겟 추적 로직
	if (mCurrentTarget)
	{
		if (mHomingStopRange > 0.0f)
		{
			XMFLOAT3 MyLoc = GetActorLocation();
			XMFLOAT3 TargetLoc;
			if(mCurrentWaypointIndex < mFinalWaypoints.size() - 1)
			{
				TargetLoc = mFinalWaypoints[mCurrentWaypointIndex];
			}
			else
			{
				TargetLoc = mCurrentTarget->GetActorLocation();
			}

			XMVECTOR VToTarget = XMLoadFloat3(&TargetLoc) - XMLoadFloat3(&MyLoc);
			float Dist = XMVectorGetX(XMVector3Length(VToTarget));

			// 2. 도착 판정 (StopRange)
			if (Dist <= mHomingStopRange)
			{
				if (mbUseWaypoints && mCurrentWaypointIndex < mFinalWaypoints.size() - 1)
				{
					mCurrentWaypointIndex++; // 다음 경유지로
					if (mCurrentWaypointIndex == mFinalWaypoints.size() - 1)
					{
						ProjComp->SetHomingTarget(mCurrentTarget->GetRootComponent());
					}
					else
					{
						ProjComp->SetHomingLocation(mFinalWaypoints[mCurrentWaypointIndex]);
					}
				}
				else
				{
					// 최종 목적지 도착 -> 호밍 해제
					if (mbForgetPreviousTarget)
					{
						mVisitedTargets.insert(mCurrentTarget);
					}

					mCurrentTarget = nullptr;
					ProjComp->SetHoming(false);
					return;
				}
			}
		}
	}
	else if(mHomingStrategy != EHomingStrategy::None)// 3. 새로운 타겟 탐색
	{
		if (AActor* NewTarget = FindBestHomingTarget())
		{
			// 이전에 방문했던 타겟인지 체크
			if (mVisitedTargets.find(NewTarget) == mVisitedTargets.end())
			{
				SetHomingTarget(NewTarget);
			}
		}
	}
}

AActor* AProjectileBase::FindBestHomingTarget()
{
	TArray<AActor*> Ignore;
	Ignore.push_back(this);
	TArray<FHitResult> Hits;

	// 1. 주변 액터 수집
	GetWorld()->SphereOverlap(GetActorLocation(), mHomingRange, Ignore, Hits, false);

	if (mHomingStrategy == EHomingStrategy::Nearest)
	{
		return FindHomingTarget_Nearest(Hits);
	}
	else if (mHomingStrategy == EHomingStrategy::Angle)
	{
		return FindHomingTarget_Angle(Hits);
	}

	return nullptr;
}

AActor* AProjectileBase::FindHomingTarget_Nearest(const TArray<FHitResult>& Hits)
{
	float ClosestDistSq = FLT_MAX;
	AActor* Target = nullptr;
	for (auto& Hit : Hits)
	{
		auto Candidate = Hit.Actor.lock();
		if (!IsHomingTarget(Candidate.get()))
		{
			continue;
		}

		XMFLOAT3 CurrLoc = GetActorLocation();
		XMFLOAT3 TargetLoc = Candidate->GetActorLocation();
		float DistSq = XMVectorGetX(XMVector3LengthSq(XMLoadFloat3(&TargetLoc) - XMLoadFloat3(&CurrLoc)));
		if (DistSq < ClosestDistSq)
		{
			ClosestDistSq = DistSq;
			Target = Candidate.get();
		}
	}
	return Target;
}

AActor* AProjectileBase::FindHomingTarget_Angle(const TArray<FHitResult>& Hits)
{
	float BestDot = -1.0f;
	XMFLOAT3 Forward = GetForwardVector();
	XMVECTOR ForwardV = XMLoadFloat3(&Forward);

	AActor* Target = nullptr;
	for (auto& Hit : Hits)
	{
		auto Candidate = Hit.Actor.lock();
		if (!IsHomingTarget(Candidate.get()))
		{
			continue;
		}

		XMFLOAT3 CandidateLoc = Candidate->GetActorLocation();
		XMFLOAT3 CurrLoc = GetActorLocation();
		XMVECTOR ToTarget = XMVector3Normalize(XMLoadFloat3(&CandidateLoc) - XMLoadFloat3(&CurrLoc));
		float Dot = XMVectorGetX(XMVector3Dot(ForwardV, ToTarget));

		// 내적값을 각도로 변환하여 범위 체크
		float Angle = XMConvertToDegrees(acosf(fmaxf(-1.0f, fminf(1.0f, Dot))));
		if (Angle <= mHomingAngle && Dot > BestDot)
		{
			BestDot = Dot;
			Target = Candidate.get();
		}
	}

	return Target;
}

bool AProjectileBase::IsHomingTarget(AActor* Actor) const
{
	if (!Actor) return false;

	// 1. 방문 기록 확인
	if (mbForgetPreviousTarget && mVisitedTargets.count(Actor)) return false;

	// 2. 태그 확인 (액터의 태그 중 하나라도 mHomingTargetTags에 있는지)
	bool bMatch = false;

	for (const std::string& Tag : mHomingTargetTags)
	{
		if (Actor->HasTag(Tag))
		{
			bMatch = true;
			break;
		}
	}

	return bMatch;
}

void AProjectileBase::GenerateWaypoints(AActor* Target)
{
	if (!mbUseWaypoints || !Target) return;

	WWorld* World = GetWorld();

	mFinalWaypoints.clear();
	mCurrentWaypointIndex = 0;

	XMFLOAT3 MyLoc = GetActorLocation();
	XMFLOAT3 TargetLoc = Target->GetActorLocation();

	XMVECTOR VStart = XMLoadFloat3(&MyLoc);
	XMVECTOR VTarget = XMLoadFloat3(&TargetLoc);
	XMVECTOR VBase = (mWaypointBase == "Actor") ? VStart : VTarget;
	World->DrawDebugLine(MyLoc, TargetLoc, XMFLOAT4(1, 0, 0, 1), 10);
	XMVECTOR VForward = VTarget - VStart;
	float TotalDist = XMVectorGetX(XMVector3Length(VForward));
	VForward = XMVector3Normalize(VForward);

	// 1. 방향 기반 공간(Direction Space)일 때만 기저 벡터 계산
	XMVECTOR VUp = XMVectorSet(0, 1, 0, 0); // 기본 WorldUp
	XMVECTOR VRight = XMVectorSet(1, 0, 0, 0);

	if (mWaypointSpace == "Direction")
	{
		XMVECTOR VWorldUp = XMVectorSet(0, 1, 0, 0);
		float Dot = fabsf(XMVectorGetX(XMVector3Dot(VForward, VWorldUp)));

		if (Dot > 0.99f)
		{
			XMVECTOR VAltUp = XMVectorSet(0, 0, 1, 0);
			VRight = XMVector3Normalize(XMVector3Cross(VAltUp, VForward));
		}
		else
		{
			VRight = XMVector3Normalize(XMVector3Cross(VWorldUp, VForward));
		}
		VUp = XMVector3Cross(VForward, VRight);
	}

	// 2. 경유지 생성 루프
	for (const auto& Offset : mConfigWaypoints)
	{
		XMVECTOR WpPos = VBase;
		XMVECTOR VOffset = XMLoadFloat3(&Offset);

		if (mWaypointSpace == "Direction")
		{
			// 전방(x), 위(y), 우측(z) 기준으로 적용
			float Scale = (mWaypointType == "Adaptive") ? TotalDist : 1.0f;
			WpPos += VForward * (XMVectorGetX(VOffset) * Scale);
			WpPos += VUp * (XMVectorGetY(VOffset) * Scale);
			WpPos += VRight * (XMVectorGetZ(VOffset) * Scale);
		}
		else // "World" Space
		{
			// 순수 월드 축 X, Y, Z 기준으로 적용
			if (mWaypointType == "Adaptive")
			{
				WpPos += VOffset * TotalDist;
			}
			else
			{
				WpPos += VOffset;
			}
		}

		FActorSpawnParameter Param;
		XMFLOAT3 FinalPos;
		XMStoreFloat3(&FinalPos, WpPos);
		World->DrawDebugLine(MyLoc, FinalPos, XMFLOAT4(1, 0, 0, 1), 5);
		mFinalWaypoints.push_back(FinalPos);
	}

	mFinalWaypoints.push_back(TargetLoc);
}