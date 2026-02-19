#pragma once
#include "ActorComponent.h"

class WEvent
{
public:
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

class WTransitionBase
{
public:
    WTransitionBase(const std::string& Target) : mTargetStateName(Target) {}
    virtual ~WTransitionBase() = default;

    // 이 전이가 실행 가능한지 체크 (다형성)
    virtual bool CanTransit() const = 0;

    const std::string& GetTargetState() const { return mTargetStateName; }

    // 전이 시 실행할 액션들 (OnEnter와 별개로 전이 자체에 귀속된 액션)
    void AddAction(const std::function<void()>& Action) { mActions.push_back(Action); }
    void ExecuteActions() const { for (auto& A : mActions) A(); }

protected:
    std::string mTargetStateName;
    std::vector<std::function<void()>> mActions;
};

// 1. Target + Event (+ Condition) 조합
class WEventTransition : public WTransitionBase
{
public:
    WEventTransition(const std::string& Target, std::function<bool()> Cond = nullptr)
        : WTransitionBase(Target), mCondition(Cond) {}

    virtual bool CanTransit() const override {
        return !mCondition || mCondition();
    }

private:
    std::function<bool()> mCondition;
};

// 2. Target + Condition (Immediate) 조합
class WImmediateTransition : public WTransitionBase
{
public:
    WImmediateTransition(const std::string& Target, std::function<bool()> Cond)
        : WTransitionBase(Target), mCondition(Cond) {}

    virtual bool CanTransit() const override {
        // Immediate는 반드시 조건이 있어야 함 (없으면 무한루프 위험)
        return mCondition && mCondition();
    }

private:
    std::function<bool()> mCondition;
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

    // 이벤트 태그별로 분류된 이벤트 전이들
    std::unordered_map<std::string, std::vector<TSharedPtr<WEventTransition>>> mEventTransitions;

    // 매 프레임 업데이트에서 검사할 즉시 전이들
    std::vector<TSharedPtr<WImmediateTransition>> mImmediateTransitions;

public:
    __forceinline void AddEventTransition(const std::string& Tag, TSharedPtr<WEventTransition> Trans)
    {
        mEventTransitions[Tag].push_back(Trans);
    }

    __forceinline void AddImmediateTransition(TSharedPtr<WImmediateTransition> Trans) 
    {
        mImmediateTransitions.push_back(Trans);
    }

    const std::vector<std::shared_ptr<WEventTransition>>* GetEventTransitions(const std::string& EventTag) const
    {
        auto it = mEventTransitions.find(EventTag);
        if (it != mEventTransitions.end())
        {
            return &it->second;
        }
        return nullptr;
    }

    const std::vector<std::shared_ptr<WImmediateTransition>>& GetImmediateTransitions() const
    {
        return mImmediateTransitions;
    }

    // 3. 부모 상태(Base) 가져오기
    WState* GetBaseState() const
    {
        return mBaseState;
    }

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

    virtual void Tick(float DeltaSecond) override;

public:
    WState* CreateStateOrGet(const std::string& Name, const std::string& BaseName = "");

    void SetInitialState(const std::string& Name);

    void TransitionTo(const std::string& Name);

    void SendEvent(const std::string& Tag);

    WState* GetState(const std::string& Name);

private:
    WState* mCurrentState = nullptr;
    std::unordered_map<std::string, std::unique_ptr<WState>> mStates;
};