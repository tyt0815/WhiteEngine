#include "ProjectileBase.h"
#include "World/World.h"
#include "Interface/HitInterface.h"
#include "Component/BoxComponent.h"
#include "Component/SphereComponent.h"

void ShowMessageBox(const std::string& Content);

XMFLOAT3 ParseFloat3(const std::string& String)
{
	XMFLOAT3 Float3;
	int Result = sscanf_s(String.c_str(), "(%f, %f, %f", &Float3.x, &Float3.y, &Float3.z);
	if (Result < 3)
	{
		ShowMessageBox("Invalid float3 format\n" + String);
		assert(Result == 3 && "Invalid float3 format");
	}
	
	return Float3;
}

AProjectileBase::AProjectileBase()
{
	SetTickGroup(ETickGroup::ETG_PrePhysics, ETickPriority::ETP_High);

	mProjMoveComp = CreateComponent<WProjectileMovementComponent>()->GetWeakPtr<WProjectileMovementComponent>();

	////////////////////////////////////////////////////////////////////////////////////////////////
	RegisterToComponentFactory("Mesh", [this](const FBlueprintAttributesMap& Attributes)
		{
			WStaticMeshComponent* Comp = this->CreateComponent<WStaticMeshComponent>();

			if (Attributes.count("Asset") > 0) Comp->SetStaticMesh(Attributes.at("Asset"));
			if (Attributes.count("Loc") > 0)
			{
				XMFLOAT3 Loc = ParseFloat3(Attributes.at("Loc"));
				Comp->SetLocalLocation(Loc);
			}
			if (Attributes.count("Rot") > 0)
			{
				XMFLOAT3 Rot = ParseFloat3(Attributes.at("Rot"));
				Comp->SetLocalLocation(Rot);
			}
			if (Attributes.count("Scale") > 0)
			{
				XMFLOAT3 Scale = ParseFloat3(Attributes.at("Scale"));
				Comp->SetLocalLocation(Scale);
			}

			return Comp;
		});
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

void AProjectileBase::LoadBlueprintAttribute(const FBlueprintAttributesMap& Attributes)
{
	Super::LoadBlueprintAttribute(Attributes);

	TSharedPtr<WProjectileMovementComponent> ProjMoveComp = mProjMoveComp.lock();
	// 1. 초기 속도 (Initial Velocity)
	if (Attributes.count("InitialVelocity"))
	{
		ProjMoveComp->SetInitialVelocity(ParseFloat3(Attributes.at("InitialVelocity")));
	}

	// 아티스트가 "Speed"라고만 적어도 초기 속도로 인식하게 배려 (선택 사항)
	else if (Attributes.count("Speed"))
	{
		ProjMoveComp->SetInitialVelocity(ParseFloat3(Attributes.at("Speed")));
	}

	// 2. 최대 속도 (Max Speed)
	if (Attributes.count("MaxSpeed"))
	{
		ProjMoveComp->SetMaxSpeed(std::stof(Attributes.at("MaxSpeed")));
	}

	// 3. 가속도 (Acceleration)
	if (Attributes.count("Acceleration"))
	{
		ProjMoveComp->SetAcceleration(std::stof(Attributes.at("Acceleration")));
	}

	// 4. 중력 배율 (Gravity Scale)
	if (Attributes.count("GravityScale"))
	{
		ProjMoveComp->SetGravityScale(std::stof(Attributes.at("GravityScale")));
	}

	// 5. 수명 (LifeSpan)
	if (Attributes.count("LifeSpan"))
	{
		ProjMoveComp->SetLifeSpan(std::stof(Attributes.at("LifeSpan")));
	}

	// 6. 유도 기능 (Homing)
	if (Attributes.count("Homing"))
	{
		// "true"/"false" 문자열을 bool로 변환
		bool bIsHoming = (Attributes.at("Homing") == "true");
		ProjMoveComp->SetHoming(bIsHoming);
	}

	if (Attributes.count("HomingTurnLimit"))
	{
		ProjMoveComp->SetHomingTurnLimit(std::stof(Attributes.at("HomingTurnLimit")));
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
