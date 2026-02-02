#pragma once
#include "Utility/Class.h"
#include <functional>
#include <unordered_map>
#include <string>
#include "Utility/Memory.h"

class WActorComponent; // 전방 선언

// 컴포넌트 생성 함수 정의
using ComponentCreator = std::function<std::shared_ptr<WActorComponent>()>;

class FComponentFactory 
{
public:
    SINGLETON(FComponentFactory);

private:
    // 내부 로직
    std::shared_ptr<WActorComponent> CreateComponent_Internal(const std::string& Name);

    void RegisterComponent_Internal(const std::string& Name, ComponentCreator Creator);

    std::unordered_map<std::string, ComponentCreator> mRegistry;

public:
    // 외부 인터페이스 (템플릿 지원)
    template<typename T>
    __forceinline static TSharedPtr<T> CreateComponent(const std::string& Name)
    {
        return Cast<T>(GetInstance()->CreateComponent_Internal(Name));
    }

    __forceinline static void RegisterComponent(const std::string& Name, ComponentCreator Creator)
    {
        GetInstance()->RegisterComponent_Internal(Name, Creator);
    }
};

// 등록용 매크로 (string 기반)
#define REGISTER_COMPONENT(ClassName) \
    inline static bool ClassName##_CompRegistered = []() { \
        FComponentFactory::RegisterComponent(#ClassName, []() { \
            return std::make_shared<ClassName>(); \
        }); \
        return true; \
    }()