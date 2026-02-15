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
    if (mCurrentState) mCurrentState->HandleEvent(Tag);
}

WState* WStateMachineComponent::GetState(const std::string& Name)
{
    auto it = mStates.find(Name);
    return (it != mStates.end()) ? it->second.get() : nullptr;
}


