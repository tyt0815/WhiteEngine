#include "TopAttackMissile.h"
#include "World/World.h"

ATopAttackMissile::ATopAttackMissile()
{
	mHitBoxComp = CreateComponent<WBoxComponent>();
	SetRootComponent(mHitBoxComp);
	if (auto BoxComp = mHitBoxComp.lock())
	{
		BoxComp->ActivatePhysicBody();
		BoxComp->SetLocalScale(XMFLOAT3(.1f, .1f, .2f));
		BoxComp->SetMotionType(EMotionType::Kinematic);
		BoxComp->SetObjectChannel(EObjectChannel::EOC_Moving);
	}

	mStaticMesh = CreateComponent<WStaticMeshComponent>();
	if (auto StaticMeshComp = mStaticMesh.lock())
	{
		StaticMeshComp->SetupAttachment(mHitBoxComp);
		StaticMeshComp->SetStaticMesh(GetStaticMeshManager()->GetStaticMesh(EStaticMeshType::ESMT_MetalCylinder));
		StaticMeshComp->SetLocalRotation(XMFLOAT3(90, 0, 0));
		StaticMeshComp->SetLocalLocation(XMFLOAT3(0, 0, GetActorScale().z));
	}

	mProjectileMovementComponent = CreateComponent<WProjectileMovementComponent>();
	if (auto ProjMoveComp = mProjectileMovementComponent.lock())
	{
		ProjMoveComp->mVelocity = XMFLOAT3(0, 0, 40);
		ProjMoveComp->SetLifeSpan(10.0f);
		ProjMoveComp->SetHoming(true);
		ProjMoveComp->SetHomingTurnLimit(360);
	}

	mTickGroup = ETickGroup::ETG_PostPhysics;
}

void ATopAttackMissile::SetTargetPosition(XMFLOAT3 Pos)
{
	DestroyPathMarkers();

	PushBackHomingPath(Pos);

	XMFLOAT3 CurrPos = GetActorLocation();
	XMVECTOR TargetPosV = XMLoadFloat3(&Pos);
	XMVECTOR CurrPosV = XMLoadFloat3(&CurrPos);
	float Dist = XMVectorGetX(XMVector3Length(XMVectorSubtract(TargetPosV, CurrPosV)));

	XMFLOAT3 TraceStart = Pos;
	XMFLOAT3 TraceEnd = TraceStart;
	TraceStart.y += 1;
	TraceEnd.y += min(Dist / 2, mMaxAltitude);
	FHitResult HitResult;

	GetWorld()->LineTrace(TraceStart, TraceEnd, HitResult, true);

	if (HitResult.HitComponent.expired())
	{
		PushFrontHomingPath(TraceEnd);
	}

	UpdateHomingPath();
}

void ATopAttackMissile::OnDestroy()
{
	DestroyPathMarkers();
	Super::OnDestroy();
}

void ATopAttackMissile::Tick(float DeltaSecond)
{
	Super::Tick(DeltaSecond);

	XMFLOAT3 Start = GetActorLocation();
	XMFLOAT3 Forward = GetFowardVector();
	XMFLOAT3 End = Start;
	Start.x += Forward.x;
	Start.y += Forward.y;
	Start.z += Forward.z;
	GetWorld()->DrawDebugLine(Start, End, XMFLOAT4(0, 1, 1, 1), 0);

	// UpdateHomingTarget
	if (!mHomingPathMarkerDeque.empty())
	{
		XMFLOAT3 CurrPos = GetActorLocation();
		XMFLOAT3 TargetPos = mHomingPathMarkerDeque.front().lock()->GetActorLocation();
		XMVECTOR CurrPosV = XMLoadFloat3(&CurrPos);
		XMVECTOR TargetPosV = XMLoadFloat3(&TargetPos);

		if (XMVectorGetX(XMVector3LengthSq(XMVectorSubtract(TargetPosV, CurrPosV))) < mArrivalThresholdSq)
		{
			NextHomingPath();
		}
	}
}

void ATopAttackMissile::PushFrontHomingPath(XMFLOAT3 Loc)
{
	TSharedPtr<AActor> Maker = GetWorld()->SpawnActor<AActor>().lock();
	Maker->SetActorLocation(Loc);
	mHomingPathMarkerDeque.push_front(Maker);
}

void ATopAttackMissile::PushBackHomingPath(XMFLOAT3 Loc)
{
	TSharedPtr<AActor> Maker = GetWorld()->SpawnActor<AActor>().lock();
	Maker->SetActorLocation(Loc);
	mHomingPathMarkerDeque.push_back(Maker);
}

void ATopAttackMissile::NextHomingPath()
{
	if (mHomingPathMarkerDeque.empty())
	{
		return;
	}

	if (TSharedPtr<AActor> Marker = mHomingPathMarkerDeque.front().lock())
	{
		Marker->Destroy();
	}
	mHomingPathMarkerDeque.pop_front();

	UpdateHomingPath();
}

void ATopAttackMissile::UpdateHomingPath()
{
	if (auto ProjComp = mProjectileMovementComponent.lock())
	{
		if (mHomingPathMarkerDeque.empty())
		{
			ProjComp->SetHomingTarget(TWeakPtr<WSceneComponent>());
		}
		else
		{
			ProjComp->SetHomingTarget(mHomingPathMarkerDeque.front().lock()->GetRootComponent());
		}
	}
}

TWeakPtr<AActor> ATopAttackMissile::GetCurrentHomingTarget() const
{
	if (mHomingPathMarkerDeque.empty())
	{
		return TWeakPtr<AActor>();
	}

	return mHomingPathMarkerDeque.front();
}

void ATopAttackMissile::DestroyPathMarkers()
{
	for (auto MarkerWeak : mHomingPathMarkerDeque)
	{
		if (auto Marker = MarkerWeak.lock())
		{
			Marker->Destroy();
		}
	}
}

