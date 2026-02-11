#include "CollisionGenerator.h"
#include "Component/ActorComponent.h"

void FCollisionGeneratorBase::AddTargetTags(const TArray<std::string>& InTargetTags)
{
    mTargetTags.insert(InTargetTags.begin(), InTargetTags.end());
}

void FCollisionGeneratorBase::AddTargetTags(const std::set<std::string>& InTargetTags)
{
    mTargetTags.insert(InTargetTags.begin(), InTargetTags.end());
}

void FCollisionGeneratorBase::UpdateActorsToIgnore(float DeltaSecond)
{
    bool bChanged = false;
    for (auto Iter = mIgnoreTimers.begin(); Iter != mIgnoreTimers.end(); )
    {
        Iter->second -= DeltaSecond;
        if (Iter->second <= 0.0f)
        {
            Iter = mIgnoreTimers.erase(Iter);
            bChanged = true;
        }
        else
        {
            ++Iter;
        }
    }
    if (bChanged)
    {
        UpdateCachedIgnoreList();
    }
}

void FCollisionGeneratorBase::UpdateCachedIgnoreList()
{
    mCachedIgnoreList.clear();
    for (const auto& KeyValue : mIgnoreTimers)
    {
        mCachedIgnoreList.push_back(KeyValue.first);
    }
}

void FCollisionGeneratorBase::ProcessHit(const FHitResult& Hit)
{
    auto HittedActor = Hit.Actor.lock();
    if (!HittedActor)
    {
        return;
    }

    // 타겟 태그를 가지고 있는지 확인
    if (mTargetTags.size() > 0)
    {
        bool bHasTag = false;
        for (const std::string& TargetTag : mTargetTags)
        {
            if (HittedActor->HasTag(TargetTag))
            {
                bHasTag = true;
                break;
            }
        }
        if (!bHasTag)
        {
            return;
        }
    }

    // 1. 이미 무시 목록에 있는지 확인
    if (mIgnoreTimers.find(HittedActor.get()) != mIgnoreTimers.end())
    {
        return;
    }

    // 2. 최대 타격 횟수 체크
    if (mMaxHit > 0 && mHitCount >= mMaxHit)
    {
        return;
    }

    // 3. 지연 시간 등록 및 카운트 증가
    if (mHitDelay > 0.0f)
    {
        mIgnoreTimers[HittedActor.get()] = mHitDelay;
        UpdateCachedIgnoreList();
    }
    mHitCount++;

    if (WSceneComponent* Comp = dynamic_cast<WSceneComponent*>(this))
    {
        // 4. 이벤트 방송
        mOnCollision.Broadcast(
            Comp,
            Hit.HitComponent.lock().get(),
            Hit.ImpactPoint,
            Hit.Normal,
            Hit.Distance,
            mDamage
        );

        if (mMaxHit > 0 && mHitCount >= mMaxHit)
        {

            Comp->Deactivate();

        }
    }
}
