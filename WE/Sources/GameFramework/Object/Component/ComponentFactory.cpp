#include "ComponentFactory.h"
#include "ActorComponent.h" // 실제 컴포넌트 헤더
#include <cassert>

FComponentFactory::FComponentFactory() {}
FComponentFactory::~FComponentFactory() {}

std::shared_ptr<WActorComponent> FComponentFactory::CreateComponent_Internal(const std::string& Name)
{
    auto it = mRegistry.find(Name);
    if (it != mRegistry.end())
    {
        return it->second();
    }

    assert(false && "Component Class not registered in Factory!");
    return nullptr;
}

void FComponentFactory::RegisterComponent_Internal(const std::string& Name, ComponentCreator Creator)
{
    mRegistry[Name] = Creator;
}