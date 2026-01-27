#include "TopAttackMissile.h"
#include "World/World.h"

ATopAttackMissile::ATopAttackMissile()
{
	mTickGroup = ETickGroup::ETG_PostPhysics;

	mHitBoxComp = CreateComponent<WBoxComponent>();
	if (auto BoxComp = mHitBoxComp.lock())
	{
		BoxComp->SetupAttachment(GetRootComponent());
		BoxComp->ActivatePhysicBody();
		BoxComp->SetLocalScale(XMFLOAT3(.1f, .1f, .2f));
		BoxComp->SetMotionType(EMotionType::Kinematic);
		BoxComp->SetObjectChannel(EObjectChannel::EOC_Moving);
		BoxComp->GenerateOverlapEvent();
	}

	mStaticMesh = CreateComponent<WStaticMeshComponent>();
	if (auto StaticMeshComp = mStaticMesh.lock())
	{
		StaticMeshComp->SetupAttachment(mHitBoxComp);
		StaticMeshComp->SetStaticMesh(GetStaticMeshManager()->GetStaticMesh(EStaticMeshType::ESMT_MetalCylinder));
		StaticMeshComp->SetLocalRotation(XMFLOAT3(90, 0, 0));
	}

	mProjectileMovementComponent = CreateComponent<WProjectileMovementComponent>();
	if (auto ProjMoveComp = mProjectileMovementComponent.lock())
	{
		ProjMoveComp->mVelocity = XMFLOAT3(0, 0, 20);
		ProjMoveComp->SetLifeSpan(10.0f);
		ProjMoveComp->SetHoming(true);
		ProjMoveComp->SetHomingTurnLimit(720);
	}

	mObjAnimComp = CreateComponent<WObjectAnimComponent>();
	if (auto AnimComp = mObjAnimComp.lock())
	{
		AnimComp->LoadKeyframesFromOADAsset(L"OAD_MissileTrack");

		TArray<std::string> AnimSamplerList;
		AnimComp->GetObjectAnimSamplerList(AnimSamplerList);
		mMissileAnimSamplers.resize(AnimSamplerList.size());
		for (int i = 0; i < mMissileAnimSamplers.size(); ++i)
		{
			mMissileAnimSamplers[i] = AnimComp->GetObjectAnimSampler(AnimSamplerList[i]);
		}

		mAnimFrameEnd = AnimComp->GetFrameEnd();
	}

	
}

void ATopAttackMissile::OnDestroy()
{
	DestroyPathMarkers();
	Super::OnDestroy();
}

void ATopAttackMissile::Tick(float DeltaSecond)
{
	Super::Tick(DeltaSecond);

	mAnimElapsedTime += DeltaSecond;

	XMFLOAT3 Start = GetActorLocation();
	XMFLOAT3 Forward = GetForwardVector();
	XMFLOAT3 End = Start;
	Start.x += Forward.x;
	Start.y += Forward.y;
	Start.z += Forward.z;
	GetWorld()->DrawDebugLine(Start, End, XMFLOAT4(0, 1, 1, 1), 0);

	// 액터 자체에 회전 변화
	{
		XMFLOAT3 Forward = GetForwardVector();
		XMVECTOR ForwardV = XMLoadFloat3(&Forward);
		XMVECTOR RotationQuatV = XMQuaternionRotationAxis(ForwardV, XMConvertToRadians(mRotationZStep) * DeltaSecond);

		XMVECTOR NewForwardV = XMVector3Rotate(ForwardV, RotationQuatV);
		XMFLOAT3 Up = GetUpVector();
		XMVECTOR NewUpV = XMVector3Rotate(XMLoadFloat3(&Up), RotationQuatV);
		XMFLOAT3 Right = GetRightVector();
		XMVECTOR NewRightV = XMVector3Rotate(XMLoadFloat3(&Right), RotationQuatV);

		XMFLOAT3 NewRotation = FDXMath::GetEulerRotationFromVectors(NewForwardV, NewRightV, NewUpV);
		SetActorRotation(NewRotation);
	}

	// 다음 호밍 타겟 찾기
	float DistanceToTargetSq = 0;
	if (auto CurrHomingTarget = GetCurrentHomingTarget().lock())
	{
		XMFLOAT3 CurrPos = GetActorLocation();
		XMFLOAT3 TargetPos = CurrHomingTarget->GetActorLocation();
		XMVECTOR CurrPosV = XMLoadFloat3(&CurrPos);
		XMVECTOR TargetPosV = XMLoadFloat3(&TargetPos);
		DistanceToTargetSq = XMVectorGetX(XMVector3LengthSq(XMVectorSubtract(TargetPosV, CurrPosV)));
		if (DistanceToTargetSq < mArrivalThresholdSq)
		{
			NextHomingPath();

			if (CurrHomingTarget = GetCurrentHomingTarget().lock())
			{
				TargetPos = CurrHomingTarget->GetActorLocation();
				TargetPosV = XMLoadFloat3(&TargetPos);
				DistanceToTargetSq = XMVectorGetX(XMVector3LengthSq(XMVectorSubtract(TargetPosV, CurrPosV)));
			}
		}	
	}

	if (auto Box = mHitBoxComp.lock())
	{
		if (mCurrAnimSampler)
		{
			float Alpha = DistanceToTargetSq / mExpectedHomingDistanceSq;
			float TargetFrame = mAnimFrameEnd * Alpha;

			XMFLOAT3 LocalLoc;
			LocalLoc.x = mCurrAnimSampler->SampleLocationX(TargetFrame);
			LocalLoc.y = mCurrAnimSampler->SampleLocationY(TargetFrame);
			LocalLoc.z = 0;

			Box->SetLocalLocation(LocalLoc);
		}
		else
		{
			Box->SetLocalLocation(XMFLOAT3(0, 0, 0));
		}
	}
}

void ATopAttackMissile::BeginPlay()
{
	Super::BeginPlay();

	if (auto Box = mHitBoxComp.lock())
	{
		Box->mOnBeginOverlapDelegate.Bind(this, &ATopAttackMissile::OnBoxOverlap);
	}
}

void ATopAttackMissile::SetTargetPosition(XMFLOAT3 Pos)
{
	DestroyPathMarkers();

	PushBackHomingPath(Pos);

	XMFLOAT3 CurrPos = GetActorLocation();
	XMVECTOR TargetPosV = XMLoadFloat3(&Pos);
	XMVECTOR CurrPosV = XMLoadFloat3(&CurrPos);
	XMVECTOR ToTargetV = XMVectorSubtract(TargetPosV, CurrPosV);
	float Dist = XMVectorGetX(XMVector3Length(ToTargetV));

	XMFLOAT3 TraceStart = Pos;
	XMVECTOR TraceStartV = XMLoadFloat3(&TraceStart);
	TraceStart.y += 1;
	XMVECTOR ToTargetN = XMVector3Normalize(ToTargetV);
	XMVECTOR UpN = XMVectorSet(0, 1, 0, 0);
	float Radian = XMVectorGetX(XMVector3AngleBetweenNormals(UpN, ToTargetN));
	if (XMConvertToRadians(90) <= Radian && Radian < 175)
	{
		XMVECTOR RightN = XMVector3Normalize(XMVector3Cross(UpN, ToTargetN));
		XMVECTOR ForwardN = XMVector3Normalize(XMVector3Cross(RightN, UpN));
		
		XMVECTOR TraceEndV = XMVectorAdd(
			TraceStartV,
			XMVectorAdd(
				XMVectorMultiply(
					RightN, 
					XMVectorReplicate(FDXMath::RandF(mMinHomingPathOffset.x, mMaxHomingPathOffset.x))
				),
				XMVectorAdd(
					XMVectorMultiply(
						UpN,
						XMVectorReplicate(FDXMath::Clamp(Dist / 2.0f, mMinHomingPathOffset.y, mMaxHomingPathOffset.y))
					),
					XMVectorMultiply(
						ForwardN,
						XMVectorReplicate(FDXMath::RandF(mMinHomingPathOffset.z, mMaxHomingPathOffset.z))
					)
				)
			)
		);
		XMFLOAT3 TraceEnd;
		XMStoreFloat3(&TraceEnd, TraceEndV);
		FHitResult HitResult;

		GetWorld()->LineTrace(TraceStart, TraceEnd, HitResult, false);

		GetWorld()->DrawDebugLine(Pos, TraceStart, XMFLOAT4(1, 0, 0, 1), 5);

		if (HitResult.HitComponent.expired())
		{
			PushFrontHomingPath(TraceEnd);
		}
	}

	UpdateHomingPath();
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
	// Path 교체
	if (auto ProjComp = mProjectileMovementComponent.lock())
	{
		if (mHomingPathMarkerDeque.empty())
		{
			ProjComp->SetHomingTarget(TWeakPtr<WSceneComponent>());
		}
		else
		{
			ProjComp->SetHomingTarget(mHomingPathMarkerDeque.front().lock()->GetRootComponent());

			if (auto Target = GetCurrentHomingTarget().lock())
			{
				XMFLOAT3 CurrLoc = GetActorLocation();
				XMFLOAT3 TargetLoc = Target->GetActorLocation();
				
				mExpectedHomingDistanceSq = 
					XMVectorGetX(
						XMVector3LengthSq(
							XMVectorSubtract(XMLoadFloat3(&CurrLoc), XMLoadFloat3(&TargetLoc))
						)
					) - mArrivalThresholdSq;
			}
			

			// 애니메이션 교체
			mAnimElapsedTime = 0;
			if (mMissileAnimSamplers.size() > 0)
			{
				int i = FDXMath::Rand(0, (int)mMissileAnimSamplers.size() - 1);
				mCurrAnimSampler = mMissileAnimSamplers[i];
			}
		}
	}

	mRotationZStep = FDXMath::RandF(mMinRotationZStep, mMaxRotationZStep);
	mArrivalThresholdSq = FDXMath::RandF(mMinArrivalThresholdSq, mMaxArrivalThresholdSq);
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

void ATopAttackMissile::OnBoxOverlap(TWeakPtr<WPhysicsComponent> Another, XMFLOAT3 ImpactPoint)
{
	// Destroy();
}

