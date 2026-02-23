#include "ProjectileMovementComponent.h"
#include "GameFramework/Object/Actor/Actor.h"
#include "GameFramework/Interface/CollisionGenerator.h"
#include "World/World.h"

WProjectileMovementComponent::WProjectileMovementComponent()
{
	RegisterWProperty("LifeSpan", &mLifeSpan);
	RegisterWProperty("MaxSpeed", &mMaxSpeed);
	RegisterWProperty("Acceleration", &mAcceleration);
	
	RegisterWProperty("ShouldBounce", &mShouldBounce);
	
	RegisterWProperty("HomingStrategy", &mHomingStrategy);
	RegisterWProperty("HomingTags", &mHomingTargetTags);
	RegisterWProperty("HomingTurnRate", &mHomingTurnLimit);
	RegisterWProperty("HomingRange", &mHomingRange);
	RegisterWProperty("HomingAngle", &mHomingAngle);
	RegisterWProperty("HomingStopRange", &mHomingStopRange);
	RegisterWProperty("RetargetTick", &mRetargetTick);
	RegisterWProperty("ForgetPreviousTarget", &mbForgetPreviousTarget);
	
	RegisterWProperty("UseWaypoints", &mbUseWaypoints);
	RegisterWProperty("Waypoints", &mConfigWaypoints);
	RegisterWProperty("WaypointSpace", &mWaypointSpace);
	RegisterWProperty("WaypointBase", &mWaypointBase);
	RegisterWProperty("WaypointType", &mWaypointType);
}

void WProjectileMovementComponent::Tick(float DeltaTime)
{
    mLifeTimeElapsed += DeltaTime;

	if (mLifeSpan > 0 && mLifeTimeElapsed > mLifeSpan)
	{
		DeactivateWithChild();
		return;
	}

    UpdateHoming(DeltaTime);

    XMFLOAT3 WorldForward = GetWorldForwardVector();
    XMVECTOR vWorldForward = XMLoadFloat3(&WorldForward);

    // 1. 중력 적용
    constexpr float GravityConstant = 9.8f;
    XMVECTOR vWorldVelocity = XMLoadFloat3(&mVelocity);
    XMVECTOR vGravity = XMVectorSet(0.0f, -GravityConstant * mGravityScale * DeltaTime, 0.0f, 0.0f);
    vWorldVelocity = XMVectorAdd(vWorldVelocity, vGravity);
    XMVECTOR vQuat = XMQuaternionIdentity();
    bool bCalcRotQuatV = true;

    // 1-1. 외부 가속도 적용 (AddForce로 누적된 값)
    XMVECTOR vExternalAccel = XMLoadFloat3(&mExternalAcceleration);
    vWorldVelocity = XMVectorAdd(vWorldVelocity, vExternalAccel);

    // 2. 호밍 로직 (속도 벡터의 방향을 꺾음)
    if (mbHomingProjectile)
    {
        XMFLOAT3 WorldLoc = GetWorldLocation();
        XMVECTOR vWorldLoc = XMLoadFloat3(&WorldLoc);
        XMFLOAT3 TargetLoc;
        if (TSharedPtr<WSceneComponent> Target = mHomingTarget.lock())
        {
            TargetLoc = Target->GetWorldLocation();
        }
        else
        {
            TargetLoc = mHomingLocation;
        }
        XMVECTOR vTargetLoc = XMLoadFloat3(&TargetLoc);
        XMVECTOR vToTarget = XMVectorSubtract(vTargetLoc, vWorldLoc);

        if (XMVectorGetX(XMVector3LengthSq(vToTarget)) > 0.00001f)
        {
            XMVECTOR CurrentDirV = XMVector3Normalize(vWorldVelocity);
            XMVECTOR ToTargetUnitV = XMVector3Normalize(vToTarget);

            // 속도 방향과 타겟 방향 사이의 각도 계산
            float Radian = XMVectorGetX(XMVector3AngleBetweenNormals(CurrentDirV, ToTargetUnitV));
            if (Radian > 0.0001f)
            {
                float TurnSpeedRad = XMConvertToRadians(mHomingTurnLimit);
                float MaxAngleThisFrame = (mHomingTurnLimit > 0) ? TurnSpeedRad * DeltaTime : Radian;
                float ActualRotation = min(Radian, MaxAngleThisFrame);

                XMVECTOR AxisV = XMVector3Normalize(XMVector3Cross(CurrentDirV, ToTargetUnitV));
                if (XMVector3Equal(AxisV, XMVectorZero()))
                {
                    XMFLOAT3 Right = GetWorldRightVector();
                    XMVECTOR RightV = XMLoadFloat3(&Right);
                    AxisV = XMVector3Normalize(XMVector3Cross(RightV, ToTargetUnitV));
                }

                vQuat = XMQuaternionRotationAxis(AxisV, ActualRotation);
                bCalcRotQuatV = false;
                // 속도 벡터 자체를 회전시킴
                vWorldVelocity = XMVector3Rotate(vWorldVelocity, vQuat);
            }
        }
        
    }

    if (bCalcRotQuatV)
    {
        XMVECTOR DirV = XMVector3Normalize(vWorldVelocity);

        float Radian = XMVectorGetX(XMVector3AngleBetweenNormals(vWorldForward, DirV));
        if (Radian > 0.0001f)
        {
            XMVECTOR AxisV = XMVector3Normalize(XMVector3Cross(vWorldForward, DirV));
            if (XMVector3Equal(AxisV, XMVectorZero()))
            {
                XMFLOAT3 Up = GetWorldUpVector();
                AxisV = XMLoadFloat3(&Up);
            }
            // 현재 방향과 속도 방향 사이의 차이만큼 회전 쿼터니언 생성
            vQuat = XMQuaternionRotationAxis(AxisV, Radian);
        }
    }

    XMVECTOR ForwardV = XMVector3Normalize(vWorldVelocity);
    if (XMVector3Equal(ForwardV, XMVectorZero()))
    {
        ForwardV = vWorldForward;
    }
    // 3. 전방 추진 가속도 적용 (스칼라)
    if (mAcceleration != 0.0f)
    {
        // 호밍/중력이 적용된 후의 '현재 진행 방향'으로 가속    
        vWorldVelocity = XMVectorAdd(vWorldVelocity, XMVectorScale(ForwardV, mAcceleration * DeltaTime));
    }

    // 4. 속도 제한 (Max Speed)
    if (mMaxSpeed >= 0.0f)
    {
        float SpeedSq = XMVectorGetX(XMVector3LengthSq(vWorldVelocity));
        if (SpeedSq > mMaxSpeed * mMaxSpeed)
        {
            vWorldVelocity = XMVectorScale(ForwardV, mMaxSpeed);
        }
    }
	
    // 5. 최종 속도 저장
    XMStoreFloat3(&mVelocity, vWorldVelocity);

    // 6. 비주얼 정렬 (투사체가 실제 이동 방향을 바라보게 함)
    if (mbOrientRotationToMovement && XMVectorGetX(XMVector3LengthSq(vWorldVelocity)) > 0.01f)
    {
        XMFLOAT4 CurrQuat = GetWorldQuatRotation();
        XMVECTOR CurrQuatV = XMLoadFloat4(&CurrQuat);
        
        XMFLOAT4 FinalQuat;
        XMStoreFloat4(&FinalQuat, XMQuaternionMultiply(CurrQuatV, vQuat));
        XMFLOAT3 FinalRotation = FDXMath::QuaternionToEuler(FinalQuat);

        SetWorldRotation(FinalRotation);
    }

    mExternalAcceleration = { 0.f, 0.f, 0.f };

    Super::Tick(DeltaTime);
}

void WProjectileMovementComponent::BeginComponent()
{
    Super::BeginComponent();
    
	XMVECTOR WorldVelocityV = XMLoadFloat3(&mVelocity);
	XMFLOAT4 CurrQuat = GetWorldQuatRotation();
	XMVECTOR CurrQuatV = XMLoadFloat4(&CurrQuat);

    // 4. 액터가 실제 날아가는 방향(월드 속도 방향)을 바라보게 회전
    if (mbOrientRotationToMovement && XMVectorGetX(XMVector3Length(WorldVelocityV)) > 0.0001f)
    {
        XMVECTOR DirV = XMVector3Normalize(WorldVelocityV);

        // 현재 액터의 전방 벡터
        XMFLOAT3 Forward = GetWorldForwardVector();
        XMVECTOR ForwardV = XMLoadFloat3(&Forward);

        // 두 벡터 사이의 각도(Radian) 구하기
        float Radian = XMVectorGetX(XMVector3AngleBetweenNormals(ForwardV, DirV));

        if (Radian > 0.0001f)
        {
            // 회전 축 구하기 (외적)
            XMVECTOR AxisV = XMVector3Normalize(XMVector3Cross(ForwardV, DirV));

            // 만약 두 벡터가 180도 반대라 외적 축이 0이 나온다면 임의의 상방 축 사용
            if (XMVector3Equal(AxisV, XMVectorZero()))
            {
                XMFLOAT3 Right = GetWorldRightVector();
                XMVECTOR RightV = XMLoadFloat3(&Right);
                AxisV = XMVector3Normalize(XMVector3Cross(RightV, DirV));
            }

            // 추가될 회전 쿼터니언 생성
            XMVECTOR DeltaQuatV = XMQuaternionRotationAxis(AxisV, Radian);

            // 기존 쿼터니언에 곱하여 최종 회전 결정 (순서: 기존 * 추가)
            XMVECTOR FinalQuatV = XMQuaternionMultiply(CurrQuatV, DeltaQuatV);

            // 4. 액터에 최종 회전 적용 (오일러 변환 후 적용)
            XMFLOAT4 FinalQuat;
            XMStoreFloat4(&FinalQuat, FinalQuatV);
            XMFLOAT3 FinalRotation = FDXMath::QuaternionToEuler(FinalQuat);

            SetWorldRotation(FinalRotation);
        }
    }
}

void WProjectileMovementComponent::SetHomingTarget(WSceneComponent* Target)
{
    if (Target == nullptr)
    {
		mHomingTarget.reset();
		SetHoming(false);
		return;
    }

	TSharedPtr<WSceneComponent> CurrTarget = mHomingTarget.lock();
	mFinalHomingTarget = CurrTarget;

	if (CurrTarget.get() == Target)
	{
		return;
	}
	mHomingTarget = Target->GetWeakPtr<WSceneComponent>();
	CurrTarget = mHomingTarget.lock();

	mOnLockon.Broadcast();

	SetHoming(true);
	if (mbUseWaypoints)
	{
		GenerateWaypoints(CurrTarget.get());
	}

	if (mFinalWaypoints.size() > 0)
	{
		mFinalHomingTarget = mHomingTarget;
		mHomingTarget.reset();
		SetHomingLocation(mFinalWaypoints.front());
	}
}

void WProjectileMovementComponent::SetHomingLocation(const XMFLOAT3& Loc)
{
	mHomingTarget.reset();
    mHomingLocation = Loc;
}

void WProjectileMovementComponent::AddForce(const XMFLOAT3& Force)
{
    XMVECTOR CurrentAccel = XMLoadFloat3(&mExternalAcceleration);
    XMVECTOR NewForce = XMLoadFloat3(&Force);
    XMStoreFloat3(&mExternalAcceleration, XMVectorAdd(CurrentAccel, NewForce));
}

void WProjectileMovementComponent::BindCollisionEvent(FCollisionGeneratorBase* CollisionGenerator)
{
	if (CollisionGenerator)
	{
		CollisionGenerator->mOnCollision.Add(this, &WProjectileMovementComponent::OnCollision);
	}
}

void WProjectileMovementComponent::OnCollision(WSceneComponent* Instigator, WPhysicsComponent* HittedComponent, XMFLOAT3 ImpulseDir, XMFLOAT3 ImpactPoint, XMFLOAT3 Normal, float Distance)
{
	if (mShouldBounce)
	{
		Bounce_Internal(Normal);
	}
}

void WProjectileMovementComponent::Bounce_Internal(XMFLOAT3 Normal)
{
	// 1. 최대 바운스 횟수 체크
	if (mMaxBounces > 0 && mCurrentBounces >= mMaxBounces)
	{
		DeactivateWithChild(); // 필요 시 호출
		return;
	}

	// 2. 물리 계산을 위한 벡터 로드
	XMVECTOR vInVelocity = XMLoadFloat3(&mVelocity);
	XMVECTOR vNormal = XMLoadFloat3(&Normal);
	vNormal = XMVector3Normalize(vNormal); // 노멀 벡터 정규화 확인

	// 3. 반사 벡터 계산 (V - 2 * (V . N) * N)
	// DirectXMath의 내장 함수를 사용합니다.
	XMVECTOR vReflected = XMVector3Reflect(vInVelocity, vNormal);

	// 4. 반발 계수(Bounciness) 적용
	// 속도가 튕길 때마다 에너지를 잃도록 스케일을 조절합니다.
	vReflected = XMVectorScale(vReflected, mBounciness);

	// 5. 최종 속도 저장
	XMStoreFloat3(&mVelocity, vReflected);

	// 6. 횟수 누적
	mCurrentBounces++;

	// 7. 끼임(Stuck) 방지 보정
	// 충돌 지점에서 노멀 방향으로 아주 미세하게 밀어내어 
	// 다음 프레임에 또다시 충돌체 내부에서 시작하는 것을 방지합니다.
	XMFLOAT3 Correction;
	XMStoreFloat3(&Correction, XMVectorScale(vNormal, 0.1f));
	AddWorldOffset(Correction);

	// 8. 바운스 이벤트 알림 (사운드나 이펙트 처리용)
	mOnBounce.Broadcast(); 
}

void WProjectileMovementComponent::UpdateHoming(float DeltaSecond)
{
	
	if (auto FinalHomingTarget = mFinalHomingTarget.lock())
	{
		if (mHomingStopRange > 0.0f)
		{
			XMFLOAT3 MyLoc = GetWorldLocation();
			XMFLOAT3 TargetLoc;
			if (mCurrentWaypointIndex < mFinalWaypoints.size())
			{
				TargetLoc = mFinalWaypoints[mCurrentWaypointIndex];
			}
			else
			{
				TargetLoc = FinalHomingTarget->GetWorldLocation();
			}

			XMVECTOR VToTarget = XMLoadFloat3(&TargetLoc) - XMLoadFloat3(&MyLoc);
			float Dist = XMVectorGetX(XMVector3Length(VToTarget));

			// 2. 도착 판정 (StopRange)
			if (Dist <= mHomingStopRange)
			{
				if (mbUseWaypoints && mCurrentWaypointIndex < mFinalWaypoints.size())
				{
					mCurrentWaypointIndex++;
					if (mCurrentWaypointIndex == mFinalWaypoints.size())
					{
						mHomingTarget = mFinalHomingTarget;
					}
					else
					{
						SetHomingLocation(mFinalWaypoints[mCurrentWaypointIndex]);
					}
				}
				else
				{
					// 최종 목적지 도착 -> 호밍 해제
					if (mbForgetPreviousTarget)
					{
						mVisitedTargets.insert(FinalHomingTarget->GetOwner<AActor>());
					}

					FinalHomingTarget = nullptr;
					SetHoming(false);
					return;
				}
			}
		}
	}
	else if (mHomingStrategy != "None")// 3. 새로운 타겟 탐색
	{
		bool bHomingFail = true;
		if (AActor* NewTarget = FindBestHomingTarget())
		{
			// 이전에 방문했던 타겟인지 체크
			if (mVisitedTargets.find(NewTarget) == mVisitedTargets.end())
			{
				bHomingFail = false;
				SetHomingTarget(NewTarget->GetRootComponent());
			}
		}
		if (bHomingFail)
		{
			mOnHomingFail.Broadcast();
		}
		else
		{
			mOnHomingSuccess.Broadcast();
		}
	}
}

AActor* WProjectileMovementComponent::FindBestHomingTarget()
{
	TArray<AActor*> Ignore;
	Ignore.push_back(GetOwner<AActor>());
	TArray<FHitResult> Hits;

	// 1. 주변 액터 수집
	GetWorld()->SphereOverlap(GetWorldLocation(), mHomingRange, Ignore, Hits, false, 0);

	if (Hits.empty())
	{
		return nullptr;
	}

	if (mHomingStrategy == "Nearest")
	{
		return FindHomingTarget_Nearest(Hits);
	}
	else if (mHomingStrategy == "Angle")
	{
		return FindHomingTarget_Angle(Hits);
	}

	return nullptr;
}

AActor* WProjectileMovementComponent::FindHomingTarget_Nearest(const TArray<FHitResult>& Hits)
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

		XMFLOAT3 CurrLoc = GetWorldLocation();
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

AActor* WProjectileMovementComponent::FindHomingTarget_Angle(const TArray<FHitResult>& Hits)
{
	float BestDot = -1.0f;
	XMFLOAT3 Forward = GetWorldForwardVector();
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
		XMFLOAT3 CurrLoc = GetWorldLocation();
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

bool WProjectileMovementComponent::IsHomingTarget(AActor* Actor) const
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

void WProjectileMovementComponent::GenerateWaypoints(WSceneComponent* Target)
{
	if (!mbUseWaypoints || !Target) return;

	WWorld* World = GetWorld();

	mFinalWaypoints.clear();
	mCurrentWaypointIndex = 0;
	assert(mHomingStopRange > 0);

	XMFLOAT3 MyLoc = GetWorldLocation();
	XMFLOAT3 TargetLoc = Target->GetWorldLocation();

	XMVECTOR VStart = XMLoadFloat3(&MyLoc);
	XMVECTOR VTarget = XMLoadFloat3(&TargetLoc);
	XMVECTOR VBase = (mWaypointBase == "Actor") ? VStart : VTarget;
	XMVECTOR VToTarget = VTarget - VStart;
	float TotalDist = XMVectorGetX(XMVector3Length(VToTarget));

	// 1. 방향 기반 공간(Direction Space)일 때만 기저 벡터 계산
	XMVECTOR VForward;
	XMVECTOR VUp;
	XMVECTOR VRight;

	if (mWaypointSpace == "Direction")
	{
		VForward = XMVector3Normalize(VToTarget);

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
	else if (mWaypointSpace == "Relative")
	{
		XMFLOAT3 WorldF;
		XMFLOAT3 WorldU;
		XMFLOAT3 WorldR;
		if (mWaypointBase == "Actor")
		{
			WorldF = GetWorldForwardVector();
			WorldU = GetWorldUpVector();
			WorldR = GetWorldRightVector();
		}
		else
		{
			WorldF = Target->GetWorldForwardVector();
			WorldU = Target->GetWorldUpVector();
			WorldR = Target->GetWorldRightVector();
		}

		VForward = XMLoadFloat3(&WorldF);
		VUp = XMLoadFloat3(&WorldU);
		VRight = XMLoadFloat3(&WorldR);
	}
	else
	{
		VForward = XMVectorSet(0, 0, 1, 0);
		VUp = XMVectorSet(0, 1, 0, 0);
		VRight = XMVectorSet(1, 0, 0, 0);
	}


	float Scale = (mWaypointType == "Adaptive") ? TotalDist : 1.0f;

	XMFLOAT3 DebugStart = MyLoc;

	// 2. 경유지 생성 루프
	for (const auto& Offset : mConfigWaypoints)
	{
		XMVECTOR WpPos = VBase;
		XMVECTOR VOffset = XMLoadFloat3(&Offset);

		if (mWaypointSpace != "World")
		{
			// 전방(x), 위(y), 우측(z) 기준으로 적용
			WpPos += VForward * (XMVectorGetZ(VOffset) * Scale);
			WpPos += VUp * (XMVectorGetY(VOffset) * Scale);
			WpPos += VRight * (XMVectorGetX(VOffset) * Scale);
		}
		else // "World" Space
		{
			WpPos += VOffset * Scale;
		}

		FActorSpawnParameter Param;
		XMFLOAT3 FinalPos;
		XMStoreFloat3(&FinalPos, WpPos);
		mFinalWaypoints.push_back(FinalPos);
		GetWorld()->DrawDebugLine(DebugStart, FinalPos, XMFLOAT4(0, 1, 1, 1), 5);
		DebugStart = FinalPos;
	}

	GetWorld()->DrawDebugLine(DebugStart, TargetLoc, XMFLOAT4(0, 1, 1, 1), 5);
}