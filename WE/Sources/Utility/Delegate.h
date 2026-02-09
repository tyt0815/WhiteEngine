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
    // 함수 등록 (언리얼의 AddUObject와 유사한 역할)
    template<typename T>
    void Add(T* ObjectPtr, void(T::* ActionFunction)(Args...))
    {
        mBoundFunctions.emplace_back([=](Args... args) {
            (ObjectPtr->*ActionFunction)(args...);
            });
    }

    // 등록된 모든 함수 실행 (언리얼의 Broadcast와 동일)
    void Broadcast(Args... args)
    {
        for (auto& func : mBoundFunctions)
        {
            if (func)
            {
                func(args...);
            }
        }
    }

    // 모든 바인딩 제거
    void Clear() { mBoundFunctions.clear(); }

    // 바인딩 여부 확인
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