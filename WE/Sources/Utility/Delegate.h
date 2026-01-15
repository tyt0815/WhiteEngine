#pragma once
//#include <functional>
//
//class FDelegate
//{
//public:
//	template<typename T>
//	void Bind(T* ObjectPtr, void(T::* ActionFunction)())
//	{
//		mBoundFunction = std::bind(ActionFunction, ObjectPtr);
//	}
//
//	__forceinline void Execute()
//	{
//		if (mBoundFunction)
//		{
//			mBoundFunction();
//		}
//	}
//
//private:
//	std::function<void()> mBoundFunction;
//};
//
//#define ONE_PARAM(TYPE1, TYPE2) TYPE1 Param1, TYPE2 Param2
//#define TWO_PARAM(TYPE1, TYPE2, TYPE3) TYPE1 Param1, TYPE2 Param2, TYPE3 Param3
//
//#define DECLARE_BIND_FUNCTION(PARAMS) template<typename T>\
//void Bind(T* ObjectPtr, void(T::* Function)(PARAMS))\
//{ std::function<void(PARAMS)> BoundFunction = std::bind(Function, ObjectPtr)}
//
//#define DECLARE_PUBLIC
//
//#define DECLARE_CLASS(CLASSNAME, CONTENTS) class CLASSNAME { CONTENTS };
//
//#define IMPLEMENT_DELEGATE(CLASSNAME) DECLARE_CLASS(CLASSNAME, );
//
//IMPLEMENT_DELEGATE(FTestDelegate);
//
//#define BEGIN_CLASS(CLASSNAME) class CLASSNAME {
//#define END_CLASS(CLASSNAME) };
//
//#define BEGIN_FUNCTION(RETURNTYPE, FUNCNAME, PARAMS) RETURNTYPE FUNCNAME(PARAMS){
//#define END_FUNCTION()};

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