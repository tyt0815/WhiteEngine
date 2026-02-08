#include "ProjectileBase.h"
#include "World/World.h"
#include "Interface/HitInterface.h"
#include "Asset/BlueprintAsset.h"
#include "Component/BoxComponent.h"
#include "Component/SphereComponent.h"
#include "Component/SplineCollisionComponent.h"

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
				Comp->GenerateCapsuleCollision(Segment, bUseBoundingBox);
			}
			

			Comp->OnCollision.Add(this, &AProjectileBase::OnCollision);

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

void AProjectileBase::PlayParticle(const std::string& Name)
{
	if (Name == "P_Explosion")
	{
		XMFLOAT3 Location = GetActorLocation();
		float Radius = 5;
		XMFLOAT4 Color = { 1, 1, 0, 1 };
		float Life = 0.5f;

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
	else
	{
		ShowMessageBox("Invalid particle name:\n" + Name);
	}
}

void AProjectileBase::OnCollision(const FHitResult& Hit)
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