#include "SplineCollisionComponent.h"
#include "Actor/Actor.h"
#include "World/World.h"

WSplineCollisionComponent::WSplineCollisionComponent()
{
	SetTickGroup(ETickGroup::ETG_PostPhysics, ETickPriority::ETP_High);

	REGISTER_WFUNC_2(GenerateCapsuleCollision, Segment, int, UseBoundingBox, bool);
}

constexpr float FIXED_DELTA = 1.0f / 60.0f;

void WSplineCollisionComponent::Tick(float DeltaSecond)
{
	Super::Tick(DeltaSecond);

	mElapsedTime += DeltaSecond;
	if (mElapsedTime < FIXED_DELTA)
	{
		return;
	}
	mElapsedTime = 0;

	bool bNeedCollisionCheck = true;
	TArray<AActor*> ActorsToIgnore;
	if (mbUseBoundingBox)
	{
		XMFLOAT3 Start = mBoundingBox.PrevLocation;
		XMFLOAT3 End = mBoundingBox.CenterComp->GetWorldLocation();
		FHitResult Hit;
		auto Owner = GetOwner().lock();
		GetWorld()->BoxTrace(
			Start,
			End,
			XMFLOAT3(1, 1, 1),
			Owner->GetActorRotation(),
			ActorsToIgnore,
			Hit,
			true,
			FIXED_DELTA
		);

		mBoundingBox.PrevLocation = End;

		if (Hit.Actor.expired())
		{
			bNeedCollisionCheck = false;
		}
	}

	if (bNeedCollisionCheck)
	{
		GetOwner().lock()->Destroy();
		for (FCapsuleCollider& Capsule : mCapsuleCollider)
		{
			XMFLOAT3 CurrLocation = Capsule.Comp->GetWorldLocation();
			FHitResult Hit;
			GetWorld()->CapsuleTrace(
				Capsule.PrevLocation,
				CurrLocation,
				Capsule.Radius,
				Capsule.HalfHeight,
				Capsule.Comp->GetWorldRotation(),
				ActorsToIgnore,
				Hit,
				true,
				FIXED_DELTA
			);
		}
	}

	// Update PrevLocation
	for (FCapsuleCollider& Capsule : mCapsuleCollider)
	{
		Capsule.PrevLocation = Capsule.Comp->GetWorldLocation();
	}
}

void WSplineCollisionComponent::GenerateCapsuleCollision(int Segment, bool bUseBoundingBox)
{
	int SplineLUTNum = GetSplineLUTNum();
	if (SplineLUTNum < 2)
	{
		mbUseBoundingBox = false;
		return;
	}

	TSharedPtr<AActor> Owner = GetOwner().lock();
	assert(Owner);

	Segment = FDXMath::Max(Segment, 1);
	int Step = FDXMath::Max(1, SplineLUTNum / Segment);

	int i = 0;
	int j = Step;
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

		TSharedPtr<WSceneComponent> Comp = Owner->CreateComponent<WSceneComponent>().lock();
		Comp->SetupAttachment(this);
		Comp->SetLocalTransform(Transform);

		FCapsuleCollider CapsuleCollider;
		CapsuleCollider.Comp = Comp.get();
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

	mbUseBoundingBox = bUseBoundingBox;
	if (mbUseBoundingBox)
	{
		if (mCapsuleCollider.empty())
		{
			return;
		}

		if (mBoundingBox.CenterComp == nullptr)
		{
			TSharedPtr<WSceneComponent> Comp = Owner->CreateComponent<WSceneComponent>().lock();
			Comp->SetupAttachment(this);
			mBoundingBox.CenterComp = Comp.get();
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
		mBoundingBox.PrevLocation = Center;
	}
}