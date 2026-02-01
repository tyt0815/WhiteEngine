#include "ComponentFactory.h"

FComponentFactory::FComponentFactory()
{

}

FComponentFactory::~FComponentFactory()
{

}

std::shared_ptr<WActorComponent> FComponentFactory::CreateComponent(const std::string& Name)
{
    if (mRegistry.find(Name) != mRegistry.end()) {
        return mRegistry[Name](); // 등록된 생성 함수 실행
    }
    return nullptr;
}
