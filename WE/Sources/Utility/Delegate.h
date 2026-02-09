#pragma once
#include <functional>

// 가변 인자 템플릿을 사용하여 모든 파라미터 개수를 수용하는 베이스 클래스
template<typename... Args>
class TBaseDelegate
{
public:
    template<typename T>
    void Bind(T* ObjectPtr, void(T::* ActionFunction)(Args...))
    {
        // 람다를 사용하여 std::bind의 placeholders 문제를 깔끔하게 해결
        mBoundFunction = [=](Args... args) {
            (ObjectPtr->*ActionFunction)(args...);
        };
    }

    __forceinline void Execute(Args... args)
    {
        if (mBoundFunction)
        {
            mBoundFunction(args...);
        }
    }

    void Unbind() { mBoundFunction = nullptr; }
    bool IsBound() const { return mBoundFunction != nullptr; }

private:
    std::function<void(Args...)> mBoundFunction;
};


#define DECLARE_DELEGATE(DelegateName) \
    using DelegateName = TBaseDelegate<>;

#define DECLARE_DELEGATE_OneParam(DelegateName, Param1) \
    using DelegateName = TBaseDelegate<Param1>;

#define DECLARE_DELEGATE_TwoParams(DelegateName, Param1, Param2) \
    using DelegateName = TBaseDelegate<Param1, Param2>;

#define DECLARE_DELEGATE_ThreeParams(DelegateName, Param1, Param2, Param3) \
    using DelegateName = TBaseDelegate<Param1, Param2, Param3>;

template<typename... Args>
class TMulticastDelegate
{
public:
    // 1. 기존 클래스 멤버 함수 바인딩 (유지)
    template<typename T>
    void Add(T* ObjectPtr, void(T::* ActionFunction)(Args...))
    {
        mBoundFunctions.emplace_back([=](Args... args) {
            (ObjectPtr->*ActionFunction)(args...);
            });
    }

    // 2. 람다 및 일반 함수 바인딩을 위한 추가 (새로 추가)
    // std::function<void(Args...)>와 호환되는 모든 함수 객체를 받습니다.
    void AddLambda(std::function<void(Args...)> InFunction)
    {
        if (InFunction)
        {
            mBoundFunctions.emplace_back(std::move(InFunction));
        }
    }

    // 더 범용적인 방식: 모든 호출 가능한 객체(Callable) 수용
    void AddRaw(void(*Function)(Args...))
    {
        mBoundFunctions.emplace_back(Function);
    }

    void Broadcast(Args... args)
    {
        for (auto& func : mBoundFunctions)
        {
            if (func) func(args...);
        }
    }

    void Clear() { mBoundFunctions.clear(); }
    bool IsBound() const { return !mBoundFunctions.empty(); }

private:
    std::vector<std::function<void(Args...)>> mBoundFunctions;
};

// --- 매크로 정의 ---

#define DECLARE_MULTICAST_DELEGATE(DelegateName) \
    using DelegateName = TMulticastDelegate<>;

#define DECLARE_MULTICAST_DELEGATE_OneParam(DelegateName, Param1) \
    using DelegateName = TMulticastDelegate<Param1>;

#define DECLARE_MULTICAST_DELEGATE_TwoParams(DelegateName, Param1, Param2) \
    using DelegateName = TMulticastDelegate<Param1, Param2>;

#define DECLARE_MULTICAST_DELEGATE_FiveParams(DelegateName, Param1, Param2, Param3, Param4, Param5) \
    using DelegateName = TMulticastDelegate<Param1, Param2, Param3, Param4, Param5>;