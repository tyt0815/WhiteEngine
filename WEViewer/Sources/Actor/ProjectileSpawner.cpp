#include "ProjectileSpawner.h"
#include "World/World.h"

AProjectileSpawner::AProjectileSpawner()
{
	SetTickGroup(ETickGroup::ETG_PostPhysics, ETickPriority::ETP_High);
}

void AProjectileSpawner::Tick(float DeltaSecond)
{
    if (!mIsActive) return;

    // 1. 초기 지연 시간(Delay) 처리
    if (!mHasStarted)
    {
        mDelayTimer -= DeltaSecond;
        if (mDelayTimer <= 0.0f)
        {
            mHasStarted = true;
            SpawnProjectile();
            // 첫 발사 후 다음 랜덤 간격 설정
            mIntervalTimer = GetRandomInterval();
        }
        return;
    }

    // 2. 반복 생성 처리
    mIntervalTimer -= DeltaSecond;
    if (mIntervalTimer <= 0.0f)
    {
        if (mIsLoop)
        {
            SpawnProjectile();
            // 매 발사마다 새로운 랜덤 간격 부여
            mIntervalTimer = GetRandomInterval();
        }
        else
        {
            mIsActive = false;
        }
    }
}

void AProjectileSpawner::SetupSpawner(const std::string& Name, float Delay, float Interval, bool bLoop)
{
    mProjectileName = Name;
    mDelayTimer = Delay;
    mMinInterval = mMaxInterval = Interval;
    mIsLoop = bLoop;
    mIsActive = true;
    mHasStarted = false;
}

void AProjectileSpawner::SetupSpawner(const std::string& Name, float Delay, float MinInterval, float MaxInterval, bool bLoop)
{
    mProjectileName = Name;
    mDelayTimer = Delay;
    mMinInterval = MinInterval; 
    mMaxInterval = MaxInterval;
    mIsLoop = bLoop;
    mIsActive = true;
    mHasStarted = false;
}

void AProjectileSpawner::StopSpawner()
{
    mIsActive = false;
}

float AProjectileSpawner::GetRandomInterval() const
{
    if (mMinInterval >= mMaxInterval) return mMinInterval;

    // 표준적인 float 랜덤 생성
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::uniform_real_distribution<float> dis(mMinInterval, mMaxInterval);
    return dis(gen);
}

void AProjectileSpawner::SpawnProjectile()
{
    FActorSpawnParameter Param;
    Param.Transform = GetActorTransform();
    GetWorld()->SpawnActorByFactory<AActor>(mProjectileName, Param);
}
