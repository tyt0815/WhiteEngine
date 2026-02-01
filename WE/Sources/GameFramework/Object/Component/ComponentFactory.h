#pragma once
#include "Utility/Class.h"
#include <functional>
#include <unordered_map>
#include <string>
#include "Utility/Memory.h"

class WActorComponent;

// 컴포넌트 생성 함수 정의
using ComponentCreator = std::function<std::shared_ptr<WActorComponent>()>;

class FComponentFactory {
public:
    SINGLETON(FComponentFactory);

private:
    std::unordered_map<std::string, ComponentCreator> mRegistry;

public:
    // 컴포넌트 등록
    void RegisterComponent(const std::string& Name, ComponentCreator Creator);

    // 문자열로 컴포넌트 생성
    std::shared_ptr<WActorComponent> CreateComponent(const std::string& ClassName);

    // 템플릿 버전 (편의용)
    template<typename T>
    static std::shared_ptr<T> Create(const std::string& ClassName)
    {
        return Cast<T>(GetInstance()->CreateComponent(ClassName));
    }
};

// 등록용 매크로
#define REGISTER_COMPONENT(ClassName) \
    inline static bool ClassName##_CompRegistered = []() { \
        FComponentFactory::GetInstance()->RegisterComponent(#ClassName, []() { \
            return std::make_shared<ClassName>(); \
        }); \
        return true; \
    }()