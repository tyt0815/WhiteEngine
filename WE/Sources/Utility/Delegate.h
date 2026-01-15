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


// 0개 파라미터
#define DECLARE_DELEGATE(DelegateName) \
    using DelegateName = TBaseDelegate<>;

// 1개 파라미터
#define DECLARE_DELEGATE_OneParam(DelegateName, Param1) \
    using DelegateName = TBaseDelegate<Param1>;

// 2개 파라미터
#define DECLARE_DELEGATE_TwoParams(DelegateName, Param1, Param2) \
    using DelegateName = TBaseDelegate<Param1, Param2>;

// 향후 N개로 확장하고 싶다면 아래와 같은 패턴으로 계속 추가 가능합니다.
#define DECLARE_DELEGATE_ThreeParams(DelegateName, Param1, Param2, Param3) \
    using DelegateName = TBaseDelegate<Param1, Param2, Param3>;