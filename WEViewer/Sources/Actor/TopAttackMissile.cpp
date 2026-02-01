#include "TopAttackMissile.h"
#include "World/World.h"
#include "MissileGridManager.h"
#include "Interface/HitInterface.h"

ATopAttackMissile::ATopAttackMissile()
{
	SetTickGroup(ETickGroup::ETG_PostPhysics, ETickPriority::ETP_High);

	mStaticMesh = CreateComponent<WStaticMeshComponent>();
	if (auto StaticMeshComp = mStaticMesh.lock())
	{
		StaticMeshComp->SetupAttachment(GetRootComponent());
		StaticMeshComp->SetStaticMesh(FStaticMeshManager::GetStaticMesh("SM_MetalCylinder"));
		StaticMeshComp->SetLocalScale(XMFLOAT3(.1f, .2f, .1f));
		StaticMeshComp->SetLocalRotation(XMFLOAT3(90, 0, 0));
	}

	mProjectileMovementComponent = CreateComponent<WProjectileMovementComponent>();
	if (auto ProjMoveComp = mProjectileMovementComponent.lock())
	{
		ProjMoveComp->mVelocity = XMFLOAT3(0, 0, 15);
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

void ATopAttackMissile::Tick(float DeltaSecond)
{
	Super::Tick(DeltaSecond);

	mAnimElapsedTime += DeltaSecond;

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

	if (auto StaticMesh = mStaticMesh.lock())
	{
		if (mCurrAnimSampler)
		{
			float Alpha = DistanceToTargetSq / mExpectedHomingDistanceSq;
			float TargetFrame = mAnimFrameEnd * Alpha;

			XMFLOAT3 LocalLoc;
			LocalLoc.x = mCurrAnimSampler->SampleLocationX(TargetFrame);
			LocalLoc.y = mCurrAnimSampler->SampleLocationY(TargetFrame);
			LocalLoc.z = 0;

			StaticMesh->SetLocalLocation(LocalLoc);
		}
		else
		{
			StaticMesh->SetLocalLocation(XMFLOAT3(0, 0, 0));
		}

		XMFLOAT3 CurrLocation = StaticMesh->GetWorldLocation();
		TArray<AActor*> ActorsToIgnore;
		FHitResult Hit;
		GetWorld()->LineTrace(mLastTickLocation, CurrLocation, ActorsToIgnore, Hit, true, .1f);

		if (!Hit.HitComponent.expired())
		{
			OnHit(Hit);
		}

		mLastTickLocation = CurrLocation;
	}
}

void ATopAttackMissile::BeginPlay()
{
	Super::BeginPlay();

	mLastTickLocation = GetActorLocation();
}

void ATopAttackMissile::OnDestroy()
{
	if (mMissileGridManager)
	{
		mMissileGridManager->RemoveMissile(this);
	}

	Super::OnDestroy();
}

void ATopAttackMissile::Initialize(const TArray<TWeakPtr<AActor>>& HomingPaths, AMissileGridManager* HitManager)
{
	for (TWeakPtr<AActor> Path : HomingPaths)
	{
		mHomingPathMarkerDeque.push_back(Path);
	}

	UpdateHomingPath();


	if (mMissileGridManager)
	{
		mMissileGridManager->RemoveMissile(this);
	}

	mMissileGridManager = HitManager;
	if (mMissileGridManager)
	{
		mMissileGridManager->AddMissile(this);
	}
}

void ATopAttackMissile::NextHomingPath()
{
	if (mHomingPathMarkerDeque.empty())
	{
		return;
	}

	mHomingPathMarkerDeque.pop_front();

	UpdateHomingPath();
}

void ATopAttackMissile::UpdateHomingPath()
{
	mRotationZStep = FDXMath::RandF(mMinRotationZStep, mMaxRotationZStep);
	mArrivalThresholdSq = FDXMath::RandF(mMinArrivalThresholdSq, mMaxArrivalThresholdSq);

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


}

TWeakPtr<AActor> ATopAttackMissile::GetCurrentHomingTarget() const
{
	if (mHomingPathMarkerDeque.empty())
	{
		return TWeakPtr<AActor>();
	}

	return mHomingPathMarkerDeque.front();
}

void ATopAttackMissile::OnHit(const FHitResult& Hit)
{
	XMVECTOR ImpactPointV = XMLoadFloat3(&Hit.ImpactPoint);
	XMVECTOR ImpactPointNormalV = XMLoadFloat3(&Hit.Normal);
	XMFLOAT3 ImpactPointWithOffset;		// ImpactPoint보다 Normal방향으로 offset을 더한 값
	XMStoreFloat3(
		&ImpactPointWithOffset,
		XMVectorMultiplyAdd(ImpactPointNormalV, XMVectorReplicate(0.1f), ImpactPointV)
	);

	if (auto StaticMesh = mStaticMesh.lock())
	{
		TArray<AActor*> ActorsToIgnore;
		TArray<FHitResult> ExplosionHits;
		TArray<JPH::ObjectLayer> ObjectChannels;
		ObjectChannels.push_back(EObjectChannel::EOC_WorldStatic);

		WWorld* World = GetWorld();

		// 폭발 반경에 액터가 있는지 체크한다.
		World->SphereOverlap(StaticMesh->GetWorldLocation(), 3, ActorsToIgnore, ExplosionHits, false, 2);
		for (int i = 0; i < ExplosionHits.size(); ++i)
		{
			if (auto Comp = ExplosionHits[i].HitComponent.lock())
			{
				// 폭발 위치와 대상자 사이에 장애물(벽)이 있는지 확인한다.
				ActorsToIgnore.clear();
				if (auto Actor = ExplosionHits[i].Actor.lock())
				{
					ActorsToIgnore.push_back(Actor.get());
				}

				XMFLOAT3 CompLocation = Comp->GetWorldLocation();
				
				FHitResult LineHit;
				World->LineTraceByObjectChannel(ImpactPointWithOffset, CompLocation, ActorsToIgnore, ObjectChannels, LineHit, false, 2);
				if (!LineHit.HitComponent.expired())
				{
					continue;
				}

				// 해당 액터가 HitInterface를 가지고 있으면, Hit를 시도한다.
				if (AActor* Owner = Comp->GetOwner())
				{
					if (auto HitInt = dynamic_cast<IHitInterface*>(Owner))
					{
						if (mMissileGridManager)
						{
							mMissileGridManager->Hit(HitInt, this);
						}
						else
						{
							HitInt->OnHit(this);
						}
					}
				}
			}
		}
	}

	if (mMissileGridManager)
	{
		mMissileGridManager->RemoveMissile(this);
	}

	Destroy();
}
