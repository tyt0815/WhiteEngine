#include "ActorFactory.h"
#include "Actor.h"
#include "Asset/AssetManager.h"
#include "Asset/BlueprintAsset.h"
#include "Utility/Debug.h"

FActorFactory::FActorFactory()
{

}

FActorFactory::~FActorFactory()
{

}

std::shared_ptr<AActor> FActorFactory::CreateActor_Internal(const std::string& Name)
{
    auto it = mRegistry.find(Name);
    if (it != mRegistry.end())
    {
        return it->second();
    }
    // 등록되지 않은 클래스일 경우 디버깅을 위해 어설트
    assert(false && "Actor Class not registered in Factory!");
    return nullptr;
}

void FActorFactory::RegisterActor_Internal(const std::string& Name, ActorCreator Creator)
{
    mRegistry[Name] = Creator;
}
