#include "StateMachineComponent.h"

WEvent* WState::GetOrCreateEvent(const std::string& Tag, const WAttributesMap& Attr)
{
    if (mEvents.find(Tag) == mEvents.end())
    {
        mEvents[Tag] = std::make_shared<WEvent>();
    }
    return mEvents[Tag].get();
}

void WState::HandleEvent(const std::string& Tag)
{
    auto it = mEvents.find(Tag);

    if (it != mEvents.end())
    {
        it->second->Dispatch();
    }
    else if (mBaseState)
    {
        mBaseState->HandleEvent(Tag);
    }
}

WStateMachineComponent::WStateMachineComponent()
{
    SetTickGroup(ETickGroup::ETG_PrePhysics, ETickPriority::ETP_Low);
}

void WStateMachineComponent::Tick(float DeltaSecond)
{
    Super::Tick(DeltaSecond);

    if (!mCurrentState) return;

    // 현재 상태부터 부모 상태(Base)까지 위로 올라가며 즉시 전이 조건 체크
    WState* Iter = mCurrentState;
    while (Iter)
    {
        for (auto& Trans : Iter->GetImmediateTransitions())
        {
            if (Trans->CanTransit())
            {
                // 전이 시 실행할 액션이 있다면 실행
                Trans->ExecuteActions();
                // 상태 전환 (TransitionTo 내부에서 mCurrentState가 바뀜)
                TransitionTo(Trans->GetTargetState());
                return; // 한 프레임에 한 번의 전이만 허용
            }
        }
        // 상속된 부모 상태로 이동하여 계속 체크
        Iter = Iter->GetBaseState();
    }
}

WState* WStateMachineComponent::CreateState(const std::string& Name, const std::string& BaseName)
{
    WState* Base = nullptr;
    if (!BaseName.empty()) Base = GetState(BaseName);

    auto NewState = std::make_unique<WState>(Name, Base);
    WState* Ptr = NewState.get();
    mStates[Name] = std::move(NewState);
    return Ptr;
}

void WStateMachineComponent::SetInitialState(const std::string& Name)
{
    TransitionTo(Name);
}

void WStateMachineComponent::TransitionTo(const std::string& Name)
{
    WState* NextState = GetState(Name);
    if (!NextState || mCurrentState == NextState) return;

    // 1. 기존 상태 탈출
    if (mCurrentState) mCurrentState->HandleEvent("OnExit");

    // 2. 상태 교체
    mCurrentState = NextState;

    // 3. 새 상태 진입
    mCurrentState->HandleEvent("OnEnter");
}

void WStateMachineComponent::SendEvent(const std::string& Tag)
{
    if (!mCurrentState) return;

    WState* Iter = mCurrentState;
    bool bTransited = false;

    // 1. 전이 규칙(Transitions) 먼저 확인
    while (Iter)
    {
        if (const auto* Transitions = Iter->GetEventTransitions(Tag))
        {
            for (auto& Trans : *Transitions)
            {
                if (Trans->CanTransit())
                {
                    Trans->ExecuteActions();
                    TransitionTo(Trans->GetTargetState());
                    bTransited = true;
                    break;
                }
            }
        }

        if (bTransited) break;
        Iter = Iter->GetBaseState(); // 부모 상태에 정의된 전이 규칙도 확인
    }

    // 2. 전이가 일어나지 않았다면, 해당 이벤트에 등록된 일반 액션 시퀀스 실행
    if (!bTransited)
    {
        mCurrentState->HandleEvent(Tag);
    }
}

WState* WStateMachineComponent::GetState(const std::string& Name)
{
    auto it = mStates.find(Name);
    return (it != mStates.end()) ? it->second.get() : nullptr;
}


