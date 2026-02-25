#pragma once

#include "Actor/Actor.h"

class AProjectileSpawner : public AActor
{
public:
    AProjectileSpawner();

    virtual void Tick(float DeltaSecond) override;

public:
    void SetupSpawner(const std::string& Name, float Delay, float Interval, bool bLoop);
    void SetupSpawner(const std::string& Name, float Delay, float MinInterval, float MaxInterval, bool bLoop);
    void StopSpawner();

private:
    float GetRandomInterval() const;
    void SpawnProjectile();

    std::string mProjectileName;

    float mDelayTimer = 0.0f;      // 첫 생성까지 남은 시간
    float mIntervalTimer = 0.0f;   // 다음 생성까지 남은 시간
    float mMinInterval = 1.0f;
    float mMaxInterval = 1.0f;

    bool mIsLoop = false;          // 반복 여부
    bool mHasStarted = false;      // 스폰 시작 여부 (Delay 체크용)
    bool mIsActive = false;        // 스패너 활성화 상태
};