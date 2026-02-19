#include "SplineCollisionComponent.h"
#include "Actor/Actor.h"
#include "World/World.h"

WSplineCollisionComponent::WSplineCollisionComponent()
{
	SetTickGroup(ETickGroup::ETG_PostPhysics, ETickPriority::ETP_High);
}

void WSplineCollisionComponent::Tick(float DeltaSecond)
{
	Super::Tick(DeltaSecond);

	// FCollisionGeneratorBase의 시간 관리 업데이트
	UpdateActorsToIgnore(DeltaSecond);

	mElapsedTime += DeltaSecond;
	if (mElapsedTime < mCollisionInterval) return;
	mElapsedTime = 0;

	// 무시 리스트 준비 (캐싱된 리스트 사용)
	TArray<AActor*> TraceIgnore = mCachedIgnoreList;
	if (auto Owner = GetOwner<AActor>()) TraceIgnore.push_back(Owner);

	bool bNeedCollisionCheck = true;
	if (mbUseBoundingBox)
	{
		FHitResult BoxHit;
		GetWorld()->BoxTrace(mBoundingBox.PrevLocation, mBoundingBox.CenterComp->GetWorldLocation(),
			XMFLOAT3(1, 1, 1), GetOwner<AActor>()->GetActorRotation(), TraceIgnore, BoxHit, mbDebug, mCollisionInterval);

		mBoundingBox.PrevLocation = mBoundingBox.CenterComp->GetWorldLocation();
		if (BoxHit.Actor.expired()) bNeedCollisionCheck = false;
	}

	for (FCapsuleCollider& Capsule : mCapsuleCollider)
	{
		XMFLOAT3 CurrLoc = Capsule.Comp->GetWorldLocation();

		if (bNeedCollisionCheck)
		{
			FHitResult Hit;
			GetWorld()->CapsuleTrace(Capsule.PrevLocation, CurrLoc, Capsule.Radius, Capsule.HalfHeight,
				Capsule.Comp->GetWorldRotation(), TraceIgnore, Hit, mbDebug, mCollisionInterval);

			if (!Hit.Actor.expired())
			{
				// 베이스 클래스의 공통 처리 로직 호출
				ProcessHit(Hit);
			}
		}
		
		Capsule.PrevLocation = CurrLoc;
	}
}

void WSplineCollisionComponent::BeginComponent()
{
	Super::BeginComponent();
}

void WSplineCollisionComponent::GenerateCollision()
{
	int SplineLUTNum = GetSplineLUTNum();
	if (SplineLUTNum < 2)
	{
		mbUseBoundingBox = false;
		return;
	}

	AActor* Owner = GetOwner<AActor>();
	assert(Owner);

	int Step = FDXMath::Max(1, SplineLUTNum / mSegment);

	int i = 0;
	int j = min(Step, SplineLUTNum - 1);
	while (true)
	{
		FTransform Transform;
		const WSplineComponent::FSplineLUT* Point0 = GetSplineLUTAt(i);
		const WSplineComponent::FSplineLUT* Point1 = GetSplineLUTAt(j);
		XMVECTOR P0 = XMLoadFloat3(&Point0->Location);
		XMVECTOR P1 = XMLoadFloat3(&Point1->Location);
		XMStoreFloat3(&Transform.Translation, (P0 + P1) / 2.0f);

		const auto Mid = GetSplineLUTAtDistanceAlongSpline((Point0->Distance + Point1->Distance) / 2.0f);
		XMVECTOR QuatV = XMLoadFloat4(&Mid.Quat);
		XMMATRIX QuatM = XMMatrixRotationQuaternion(QuatV);
		XMVECTOR RotAxisV = QuatM.r[0];
		XMFLOAT4 FinalQuat;
		XMStoreFloat4(&FinalQuat, XMQuaternionMultiply(QuatV, XMQuaternionRotationAxis(RotAxisV, XM_PI / 2.0f)));
		Transform.SetRotationByQuat(FinalQuat);

		XMVECTOR ToP1 = XMVectorSubtract(P1, P0);
		float Length = XMVectorGetX(XMVector3Length(ToP1));

		WSceneComponent* Comp = Owner->CreateComponent<WSceneComponent>();
		Comp->SetupAttachment(this);
		Comp->SetRelativeTransform(Transform);

		FCapsuleCollider CapsuleCollider;
		CapsuleCollider.Comp = Comp;
		CapsuleCollider.Radius = Mid.Property1;
		CapsuleCollider.HalfHeight = Length / 2.0f - CapsuleCollider.Radius + Mid.Property2;
		CapsuleCollider.PrevLocation = Comp->GetWorldLocation();

		mCapsuleCollider.push_back(std::move(CapsuleCollider));

		if (j >= SplineLUTNum - 1)
		{
			break;
		}
		i = j;
		j = min(j + Step, SplineLUTNum - 1);
	}

	if (mbUseBoundingBox)
	{
		if (mCapsuleCollider.empty())
		{
			return;
		}

		if (mBoundingBox.CenterComp == nullptr)
		{
			WSceneComponent* Comp = Owner->CreateComponent<WSceneComponent>();
			Comp->SetupAttachment(this);
			mBoundingBox.CenterComp = Comp;
		}

		XMVECTOR MinBound = XMVectorReplicate(FLT_MAX);
		XMVECTOR MaxBound = XMVectorReplicate(-FLT_MAX);

		for (const auto& Capsule : mCapsuleCollider)
		{
			XMVECTOR Pos = XMLoadFloat3(&Capsule.PrevLocation);
			float R = Capsule.Radius;
			float H = Capsule.HalfHeight + R; // 전체 높이

			// 캡슐의 위/아래 끝점들을 포함하도록 계산
			XMVECTOR Offset = XMVectorSet(R, H, R, 0);
			MinBound = XMVectorMin(MinBound, Pos - Offset);
			MaxBound = XMVectorMax(MaxBound, Pos + Offset);
		}
		XMFLOAT3 Center;
		XMStoreFloat3(&Center, (MinBound + MaxBound) * 0.5f);
		mBoundingBox.CenterComp->SetWorldLocation(Center);
		XMStoreFloat3(&mBoundingBox.Extent, (MaxBound - MinBound) * 0.5f);
	}
}

void WSplineCollisionComponent::OnActivate()
{
	Super::OnActivate();

	if (mbUseBoundingBox && mBoundingBox.CenterComp)
	{
		mBoundingBox.PrevLocation = mBoundingBox.CenterComp->GetWorldLocation();
	}

	for (FCapsuleCollider& Capsule : mCapsuleCollider)
	{
		Capsule.PrevLocation = Capsule.Comp->GetWorldLocation();
	}

	mElapsedTime = 0;
}

void WSplineCollisionComponent::OnDeactivate()
{
	Super::OnDeactivate();
}