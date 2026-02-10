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

AProjectileBase::AProjectileBase()
{
	SetTickGroup(ETickGroup::ETG_PrePhysics, ETickPriority::ETP_High);

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

	RegisterWComponentFactory("Movement", [this](const WAttributesMap& Attributes)
		{
			const std::string& Name = Attributes.at("Name");
			auto Comp = this->CreateComponent<WProjectileMovementComponent>();

			// 1. 이동 및 물리 속성
			ApplyAttribute<XMFLOAT3>(Attributes, "InitSpeed", [=](auto&& v) { Comp->SetInitialVelocity(v); });

			ApplyAttribute<float>(Attributes, "MaxSpeed", [=](auto&& v) { Comp->SetMaxSpeed(v); });
			RegisterWProperty(Name + ".MaxSpeed", &Comp->mMaxSpeed);

			ApplyAttribute<float>(Attributes, "Acceleration", [=](auto&& v) { Comp->SetAcceleration(v); });
			RegisterWProperty(Name + ".Acceleration", &Comp->mAcceleration);

			ApplyAttribute<float>(Attributes, "GravityScale", [=](auto&& v) { Comp->SetGravityScale(v); });
			RegisterWProperty(Name + ".GravityScale", &Comp->mGravityScale);

			ApplyAttribute<float>(Attributes, "LifeSpan", [=](auto&& v) { Comp->SetLifeSpan(v); });
			RegisterWProperty(Name + ".LifeSpan", &Comp->mLifeSpan);

			// 3. 호밍 (Homing) 속성 적용 및 등록
			ApplyAttribute<std::string>(Attributes, "HomingStrategy", [=](auto&& v) {
				if (v == "Nearest") Comp->mHomingStrategy = EHomingStrategy::Nearest;
				else if (v == "Angle") Comp->mHomingStrategy = EHomingStrategy::Angle;
				else Comp->mHomingStrategy = EHomingStrategy::None;
				});

			ApplyAttribute<std::set<std::string>>(Attributes, "HomingTags", [=](auto&& v) { Comp->mHomingTargetTags = v; });
			RegisterWProperty(Name + ".HomingTags", &Comp->mHomingTargetTags);

			ApplyAttribute<float>(Attributes, "HomingRange", [=](auto&& v) { Comp->mHomingRange = v; });
			RegisterWProperty(Name + ".HomingRange", &Comp->mHomingRange);

			ApplyAttribute<float>(Attributes, "HomingAngle", [=](auto&& v) { Comp->mHomingAngle = v; });
			RegisterWProperty(Name + ".HomingAngle", &Comp->mHomingAngle);

			ApplyAttribute<float>(Attributes, "HomingTurnRate", [=](auto&& v) { Comp->SetHomingTurnLimit(v); });
			RegisterWProperty(Name + ".HomingTurnRate", &Comp->mHomingTurnLimit);

			ApplyAttribute<float>(Attributes, "HomingRetargetTick", [=](auto&& v) { Comp->mRetargetTick = v; });
			RegisterWProperty(Name + ".RetargetTick", &Comp->mRetargetTick);

			ApplyAttribute<float>(Attributes, "HomingStopRange", [=](auto&& v) { Comp->mHomingStopRange = v; });
			RegisterWProperty(Name + ".HomingStopRange", &Comp->mHomingStopRange);

			ApplyAttribute<bool>(Attributes, "ForgetPrev", [=](auto&& v) { Comp->mbForgetPreviousTarget = v; });
			RegisterWProperty(Name + ".ForgetPrev", &Comp->mbForgetPreviousTarget);

			// 4. 웨이포인트 (Waypoint) 속성 적용 및 등록
			ApplyAttribute<bool>(Attributes, "UseWaypoint", [=](auto&& v) { Comp->mbUseWaypoints = v; });
			RegisterWProperty(Name + ".UseWaypoint", &Comp->mbUseWaypoints);

			ApplyAttribute<std::string>(Attributes, "WpSpace", [=](auto&& v) { Comp->mWaypointSpace = v; });
			RegisterWProperty(Name + ".WpSpace", &Comp->mWaypointSpace);

			ApplyAttribute<std::string>(Attributes, "WpBase", [=](auto&& v) { Comp->mWaypointBase = v; });
			RegisterWProperty(Name + ".WpBase", &Comp->mWaypointBase);

			ApplyAttribute<std::string>(Attributes, "WpType", [=](auto&& v) { Comp->mWaypointType = v; });
			RegisterWProperty(Name + ".WpType", &Comp->mWaypointType);

			ApplyAttribute<std::vector<XMFLOAT3>>(Attributes, "Waypoints", [=](auto&& v) { Comp->mConfigWaypoints = v; });
			RegisterWProperty(Name + ".Waypoints", &Comp->mConfigWaypoints);

			return Comp;
		});

	RegisterWComponentFactory("Anim", [this](auto&& Attributes) {
		auto Comp = this->CreateComponent<WObjectAnimComponent>();

		return Comp;
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

	RegisterWActionFactory("Animation", [this](const WAttributesMap& Attributes) {
		assert(Attributes.count("Target") > 0);
		const std::string& Target = Attributes.at("Target");
		WObjectAnimComponent* Anim = dynamic_cast<WObjectAnimComponent*>(GetWComponent(Target));
		assert(Anim);

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
			assert(Attributes.count("Target") > 0);
			const std::string& Target = Attributes.at("Target");
			WObjectAnimComponent* Anim = dynamic_cast<WObjectAnimComponent*>(GetWComponent(Target));
			assert(Anim);

			const std::string& PropertyName = Attributes.at("Property");
			float* Prop = std::get<float*>(GetWProperty(PropertyName));

			std::string CurveName = Attributes.at("Curve");

			bool bIsModifier = false;
			ExtractAttribute(Attributes, "Modifier", bIsModifier);

			return [=]()
			{
				Anim->BindCurve(CurveName, Prop, bIsModifier, *Prop);
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
}

void AProjectileBase::BeginPlay()
{
	Super::BeginPlay();
}

void AProjectileBase::LoadWConfigs(const std::unordered_map<std::string, WAttributesMap>& Configs)
{
	Super::LoadWConfigs(Configs);
}

void AProjectileBase::ApplyWComponentCommonAttribute(FBlueprintComponentNode* CompNode, WSceneComponent* Comp)
{
	Super::ApplyWComponentCommonAttribute(CompNode, Comp);
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