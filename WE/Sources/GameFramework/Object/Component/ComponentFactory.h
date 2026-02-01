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
    std::shared_ptr<WActorComponent> CreateComponent(const std::string& Name);

    std::unordered_map<std::string, ComponentCreator> mRegistry;

public:
    __forceinline void RegisterComponent(const std::string& Name, ComponentCreator Creator)
    {
        mRegistry[Name] = Creator;
    }

    __forceinline static std::shared_ptr<WActorComponent> CreateComponent(const std::string& Name)
    {
        GetInstance()->CreateComponent(Name);
    }
};


#define REGISTER_COMPONENT(ClassName) \
    FComponentFactory::RegisterComponent(L#ClassName, []() { return std::make_shared<ClassName>(); });