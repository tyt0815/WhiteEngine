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
	// WComponent
	////////////////////////////////////////////////////////////////////////////////////////////////

	RegisterWComponentFactory("Collision", [this](const WAttributesMap& Attributes)
		{
			WSceneComponent* Component = nullptr;
			std::string Type = Attributes.at("Type");
			if (Type == "Spline")
			{
				WSplineCollisionComponent* Comp = this->CreateComponent<WSplineCollisionComponent>();

				ApplyAttribute(Attributes, "Asset", [&](const std::string& v) {
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
				
				ApplyAttribute(Attributes, "Extent", ParseFloat3, [&](const XMFLOAT3& v) {
					Comp->SetExtent(v);
					});

				Component = Comp;
			}
			else if (Type == "Capsule")
			{
				WCapsuleCollisionComponent* Comp = this->CreateComponent<WCapsuleCollisionComponent>();

				// 반지름(Radius) 설정
				ApplyAttribute(Attributes, "Radius", ParseFloat, [&](float v) {
					float HalfHeight = 0.0f; // 기존 값을 유지하기 위해 임시 저장소 필요 시 로직 조정
					Comp->SetCapsuleSize(v, 1.0f); // 기본 HalfHeight 예시
					});

				// 반높이(HalfHeight) 설정
				ApplyAttribute(Attributes, "HalfHeight", ParseFloat, [&](float v) {
					Comp->SetCapsuleSize(1.0f, v);
					});

				Component = Comp;
			}
			else if (Type == "Sphere")
			{
				WSphereCollisionComponent* Comp = this->CreateComponent<WSphereCollisionComponent>();

				// 반지름(Radius) 설정
				ApplyAttribute(Attributes, "Radius", ParseFloat, [&](float v) {
					Comp->SetRadius(v);
					});

				Component = Comp;
			}
			else
			{
				ShowMessageBox("Invalid collision type:\n" + Type);
				assert(false);
			}

			ApplySceneComponentDefaultAttributes(Component, Attributes);

			if (FCollisionGeneratorBase* CollisionGenerator = dynamic_cast<FCollisionGeneratorBase*>(Component))
			{
				assert(CollisionGenerator);

				bool bActivate = false;
				ExtractAttribute(Attributes, "Activate", bActivate);
				if (bActivate)
				{
					CollisionGenerator->GenerateCollision();
				}

				CollisionGenerator->mOnCollision.Add(this, &AProjectileBase::OnCollision);
			}


			return Component;
		});

	RegisterWComponentFactory("SplineCollision", [this](const WAttributesMap& Attributes)
		{
			WSplineCollisionComponent* Comp = this->CreateComponent<WSplineCollisionComponent>();

			ApplySceneComponentDefaultAttributes(Comp, Attributes);

			ApplyAttribute(Attributes, "Asset", [&](const std::string& v) {
				Comp->LoadSplineFromAsset(v);
				});

			bool bActivate = false;
			ExtractAttribute(Attributes, "Activate", bActivate);
			if (bActivate)
			{
				int Segment = 1;
				ExtractAttribute(Attributes, "Segment", Segment);
				bool bUseBoundingBox = true;
				ExtractAttribute(Attributes, "BoundingBox", bUseBoundingBox);
				Comp->SetSegment(Segment);
				Comp->SetBoundingBox(bUseBoundingBox);
				Comp->GenerateCollision();
			}
			

			Comp->mOnCollision.Add(this, &AProjectileBase::OnCollision);

			return Comp;
		});

	////////////////////////////////////////////////////////////////////////////////////////////////
	// WComponent End
	////////////////////////////////////////////////////////////////////////////////////////////////

	////////////////////////////////////////////////////////////////////////////////////////////////
	// WAction
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
	// WAction End
	////////////////////////////////////////////////////////////////////////////////////////////////

	////////////////////////////////////////////////////////////////////////////////////////////////
	// WProperty
	////////////////////////////////////////////////////////////////////////////////////////////////

	RegisterWProperty("MaxSpeed", &ProjComp->mMaxSpeed);
	RegisterWProperty("Acceleration", &ProjComp->mAcceleration);
	RegisterWProperty("GravityScale", &ProjComp->mGravityScale);
	RegisterWProperty("LifeSpan", &ProjComp->mLifeSpan);
	RegisterWProperty("HomingTurnLimit", &ProjComp->mHomingTurnLimit);

	float mHomingTurnLimit = 0;

	////////////////////////////////////////////////////////////////////////////////////////////////
	// WProperty End
	////////////////////////////////////////////////////////////////////////////////////////////////

	////////////////////////////////////////////////////////////////////////////////////////////////
	// WEvent
	////////////////////////////////////////////////////////////////////////////////////////////////

	mOnHitEvent = RegisterWEvent("OnHit");

	////////////////////////////////////////////////////////////////////////////////////////////////
	// WEvent End
	////////////////////////////////////////////////////////////////////////////////////////////////
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
}

void AProjectileBase::BeginPlay()
{
	Super::BeginPlay();

	if (WProjectileMovementComponent* Comp = GetComponent<WProjectileMovementComponent>())
	{
		mProjMoveComp = Comp->GetWeakPtr<WProjectileMovementComponent>();
	}
}

void AProjectileBase::LoadWAttributes(const WAttributesMap& Attributes)
{
	Super::LoadWAttributes(Attributes);

	TSharedPtr<WProjectileMovementComponent> ProjMoveComp = mProjMoveComp.lock();

	// 1. 초기 속도 (Initial Velocity) - XMFLOAT3
	ApplyAttribute(Attributes, "InitSpeed", ParseFloat3, [&](const XMFLOAT3& v) {
		ProjMoveComp->SetInitialVelocity(v);
		});

	// 2. 최대 속도 (Max Speed) - float
	ApplyAttribute(Attributes, "MaxSpeed", ParseFloat, [&](float v) {
		ProjMoveComp->SetMaxSpeed(v);
		});

	// 3. 가속도 (Acceleration) - float
	ApplyAttribute(Attributes, "Acceleration", ParseFloat, [&](float v) {
		ProjMoveComp->SetAcceleration(v);
		});

	// 4. 중력 배율 (Gravity Scale) - float
	ApplyAttribute(Attributes, "GravityScale", ParseFloat, [&](float v) {
		ProjMoveComp->SetGravityScale(v);
		});

	// 5. 수명 (LifeSpan / Life) - float
	ApplyAttribute(Attributes, "Life", ParseFloat, [&](float v) {
		ProjMoveComp->SetLifeSpan(v);
		});

	// 6. 유도 기능 (Homing) - float
	ApplyAttribute(Attributes, "Homing", ParseFloat, [&](float v) {
		SetSmartHoming(true, v);
		});

	// 7. 유도 회전 제한 (HomingTurnLimit) - float
	ApplyAttribute(Attributes, "HomingTurnLimit", ParseFloat, [&](float v) {
		ProjMoveComp->SetHomingTurnLimit(v);
		});
}

void AProjectileBase::OnLoadWComponent(FBlueprintComponentNode* CompNode, WSceneComponent* Comp)
{
	Super::OnLoadWComponent(CompNode, Comp);

	if (CompNode->Attributes.count("Anim") && ParseBool(CompNode->Attributes["Anim"]))
	{
		WObjectAnimComponent* AnimComp = CreateComponent<WObjectAnimComponent>();
		AnimComp->SetTargetComponent(Comp);
		mWObjAnimComp[CompNode->Attributes["Name"]] = AnimComp;
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

void AProjectileBase::OnCollision(AActor* Actor, WPhysicsComponent* Comp, XMFLOAT3 ImpactPoint, XMFLOAT3 Normal, float Distance)
{
	mOnHitEvent->Dispatch();
}

//AProjectileBase::FTrackedSceneCompInfo* AProjectileBase::AddTrackedComp(WSceneComponent* Comp)
//{
//	// 1. 기존 요소 탐색
//	auto Iter = std::find_if(mTrackedComp.begin(), mTrackedComp.end(),
//		[Comp](const TUniquePtr<FTrackedSceneCompInfo>& Info)
//		{
//			return Info->Target == Comp;
//		}
//	);
//
//	// 2. 이미 존재하면 즉시 리턴
//	if (Iter != mTrackedComp.end())
//	{
//		return Iter->get();
//	}
//
//	// 3. 없으면 새로 생성 및 추가
//	auto Info = MakeUnique<FTrackedSceneCompInfo>();
//	Info->Target = Comp;
//	Info->LastTickLocation = Comp->GetWorldLocation();
//
//	// 포인터를 꺼내놓고 push_back 합니다. (std::move 이후에는 Info를 쓸 수 없으므로)
//	FTrackedSceneCompInfo* RawPtr = Info.get();
//	mTrackedComp.push_back(std::move(Info));
//
//	return RawPtr;
//}
//
//void AProjectileBase::SetTrailParticle(WSceneComponent* Comp)
//{
//	FTrackedSceneCompInfo* Info = AddTrackedComp(Comp);
//
//	if (std::count(mTrailComp.begin(), mTrailComp.end(), Info) == 0)
//	{
//		mTrailComp.push_back(Info);
//	}
//}
//
//void AProjectileBase::MakeLineCollision(WSceneComponent* Comp)
//{
//	FTrackedSceneCompInfo* Info = AddTrackedComp(Comp);
//	const auto Iter = std::find_if(
//		mCollisionInfo.begin(), mCollisionInfo.end(),
//		[Info](const FMakeCollisionInfo& A) 
//		{ 
//			return A.TargetInfo == Info; 
//		}
//	);
//	if (Iter == mCollisionInfo.end())
//	{
//		FMakeCollisionInfo CollInfo;
//		CollInfo.TargetInfo = Info;
//		CollInfo.Type = FMakeCollisionInfo::EType::ET_Line;
//		mCollisionInfo.push_back(std::move(CollInfo));
//	}
//}
//
//std::string AProjectileBase::SelectRandomString(const TArray<std::string>& Strings)
//{
//	return Strings[FDXMath::Rand(0, (int)Strings.size() - 1)];
//}
//
//void AProjectileBase::CreateBoxTraceHitBySplineComponent(WSplineComponent* SplineComponent, int Segment)
//{
//	if (SplineComponent == nullptr)
//	{
//		return;
//	}
//
//	WSceneComponent* Parent = SplineComponent->GetParent();
//	if (Parent == nullptr)
//	{
//		Parent = SplineComponent;
//	}
//
//	Segment = max(Segment, 1);
//	int SplineLUTNum = SplineComponent->GetSplineLUTNum();
//	int Step = max(1, SplineLUTNum / Segment);
//
//	int i = 0;
//	int j = Step;
//	while (true)
//	{
//		const WSplineComponent::FSplineLUT* Point0 = SplineComponent->GetSplineLUTAt(i);
//		const WSplineComponent::FSplineLUT* Point1 = SplineComponent->GetSplineLUTAt(j);
//		const auto Mid = SplineComponent->GetSplineLUTAtDistanceAlongSpline((Point0->Distance + Point1->Distance) / 2.0f);
//		
//		XMVECTOR P0 = XMLoadFloat3(&Point0->Location);
//		XMVECTOR P1 = XMLoadFloat3(&Point1->Location);
//		XMVECTOR ToP1 = XMVectorSubtract(P1, P0);
//
//		FTransform Transform;
//		XMStoreFloat3(&Transform.Translation, (P0 + P1) / 2.0f);
//
//		Transform.SetRotationByQuat(Mid.Quat);
//
//		Transform.Scale.x = max(0.1f, Mid.Property1);
//		Transform.Scale.y = max(0.1f, Mid.Property2);
//		Transform.Scale.z = max(0.1f, XMVectorGetX(XMVector3Length(ToP1)));
//
//		TSharedPtr<WSceneComponent> Comp = CreateComponent<WSceneComponent>().lock();
//		Comp->SetupAttachment(Parent);
//		Comp->SetLocalTransform(Transform);
//		mBoxCollisionInfo.push_back(AddTrackedComp(Comp.get()));
//
//		if (j >= SplineLUTNum - 1)
//		{
//			break;
//		}
//		i = j;
//		j = min(j + Step, SplineLUTNum - 1);
//	}
//}
//
//void AProjectileBase::CreateBoxColliderBySplineComponent(WSplineComponent* SplineComponent, int Segment)
//{
//	if (SplineComponent == nullptr)
//	{
//		return;
//	}
//
//	WSceneComponent* Parent = SplineComponent->GetParent();
//	if (Parent == nullptr)
//	{
//		Parent = SplineComponent;
//	}
//
//	Segment = max(Segment, 1);
//	int SplineLUTNum = SplineComponent->GetSplineLUTNum();
//	int Step = max(1, SplineLUTNum / Segment);
//
//	int i = 0;
//	int j = Step;
//	while(true)
//	{
//		const WSplineComponent::FSplineLUT* Point0 = SplineComponent->GetSplineLUTAt(i);
//		const WSplineComponent::FSplineLUT* Point1 = SplineComponent->GetSplineLUTAt(j);
//		const auto Mid = SplineComponent->GetSplineLUTAtDistanceAlongSpline((Point0->Distance + Point1->Distance) / 2.0f);
//		XMVECTOR P0 = XMLoadFloat3(&Point0->Location);
//		XMVECTOR P1 = XMLoadFloat3(&Point1->Location);
//		XMVECTOR ToP1 = XMVectorSubtract(P1, P0);
//
//		FTransform Transform;
//		XMStoreFloat3(&Transform.Translation, (P0 + P1) / 2.0f);
//		
//		Transform.SetRotationByQuat(Mid.Quat);
//
//		Transform.Scale.x = max(0.1f, Mid.Property1);
//		Transform.Scale.y = max(0.1f, Mid.Property2);
//		Transform.Scale.z = max(0.1f, XMVectorGetX(XMVector3Length(ToP1)));
//		
//
//		TSharedPtr<WBoxComponent> BoxComp = CreateComponent<WBoxComponent>().lock();
//		assert(BoxComp);
//		BoxComp->SetupAttachment(Parent);
//		BoxComp->ActivatePhysicBody();
//		BoxComp->SetExtent(XMFLOAT3(0.5f, 0.5f, 0.7f));
//		BoxComp->SetMotionType(EMotionType::Kinematic);
//		BoxComp->SetObjectChannel(EObjectChannel::EOC_Projectile);
//		BoxComp->SetLocalTransform(Transform);
//		BoxComp->GenerateOverlapEvent();
//		BoxComp->BeginComponent();
//		BoxComp->mOnBeginOverlapDelegate.Bind(this, &AProjectileBase::OnHit);
//		
//		if (j >= SplineLUTNum - 1)
//		{
//			break;
//		}
//		i = j;
//		j = min(j + Step, SplineLUTNum - 1);
//	}
//}