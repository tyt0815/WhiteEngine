#pragma once
#include "ActorComponent.h"
// #include "WEventRegistry.h"

class WEvent
{
public:
    static WEvent* GenerateTargetEvent(std::unordered_map<std::string, TSharedPtr<WEvent>>& Container, const std::string& Target);

    static WEvent* GenerateEvent(std::vector<TSharedPtr<WEvent>>& Container);

    void Dispatch() const
    {
        for (const auto& Action : mActions) Action();
    }
    void AddAction(const std::function<void()>& Action)
    {
        mActions.push_back(Action);
    }

private:
    std::vector<std::function<void()>> mActions;
};

class WState
{
public:
    WState(const std::string& Name, WState* BaseState = nullptr)
        : mName(Name), mBaseState(BaseState) {}

    // 특정 이벤트(OnEnter, Custom1 등)를 가져오거나 생성
    WEvent* GetOrCreateEvent(const std::string& Tag, const WAttributesMap& Attr = {});

    // 상태 내 이벤트 실행
    void HandleEvent(const std::string& Tag);

private:
    std::string mName;
    WState* mBaseState = nullptr; // 상속받은 부모 상태
    std::unordered_map<std::string, TSharedPtr<WEvent>> mEvents;

public:
    __forceinline const std::string& GetName() const 
    {
        return mName; 
    }
};

class WStateMachineComponent : public WActorComponent
{
    typedef WActorComponent Super;
public:
    WStateMachineComponent();

public:
    WState* CreateState(const std::string& Name, const std::string& BaseName = "");

    void SetInitialState(const std::string& Name);

    void TransitionTo(const std::string& Name);

    void SendEvent(const std::string& Tag);

    WState* GetState(const std::string& Name);

private:
    WState* mCurrentState = nullptr;
    std::unordered_map<std::string, std::unique_ptr<WState>> mStates;
};