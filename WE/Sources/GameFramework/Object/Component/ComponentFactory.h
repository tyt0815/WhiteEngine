#pragma once
#include "Utility/Class.h"
#include <functional>
#include "ActorComponent.h"

// ComponentFactory.h
using ComponentCreator = std::function<std::shared_ptr<WActorComponent>()>;

class FComponentFactory {
public:
    SINGLETON(FComponentFactory);    

private:
    std::shared_ptr<WActorComponent> CreateComponent(const std::wstring& Name);

    std::unordered_map<std::wstring, ComponentCreator> mRegistry;

public:
    __forceinline void RegisterComponent(const std::wstring& Name, ComponentCreator Creator)
    {
        mRegistry[Name] = Creator;
    }

    __forceinline static std::shared_ptr<WActorComponent> CreateComponent(const std::wstring& Name)
    {
        GetInstance()->CreateComponent(Name);
    }
};

// 컴포넌트 클래스 내부 혹은 상단
#define REGISTER_COMPONENT(ClassName) \
    FComponentFactory::RegisterComponent(L#ClassName, []() { return std::make_shared<ClassName>(); });