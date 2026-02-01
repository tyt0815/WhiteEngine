#include "ComponentFactory.h"
#include "GameFramework/Object/Component/ActorComponent.h"

FComponentFactory::FComponentFactory() {}
FComponentFactory::~FComponentFactory() {}

void FComponentFactory::RegisterComponent(const std::string& Name, ComponentCreator Creator)
{
    mRegistry[Name] = Creator;
}

std::shared_ptr<WActorComponent> FComponentFactory::CreateComponent(const std::string& ClassName)
{
    auto it = mRegistry.find(ClassName);
    if (it != mRegistry.end())
    {
        return it->second();
    }

    // 에러 로그나 어설트 추가 (디버깅용)
    assert(false && "Unknown Component Class Name!");
    return nullptr;
}