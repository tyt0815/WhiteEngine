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

std::shared_ptr<AActor> FActorFactory::CreateActorInternal(const std::string& Name)
{
    auto it = mRegistry.find(Name);
    if (it != mRegistry.end())
    {
        return it->second();
    }
    return nullptr;
}

std::shared_ptr<AActor> FActorFactory::CreateBlueprintActor_Internal(const std::wstring& BlueprintName)
{
    FBlueprintAsset* Asset = FAssetManager::GetAsset<FBlueprintAsset>(BlueprintName);
    const auto& ActorNode = Asset->mActorNode;
    assert(mRegistry.count(ActorNode.ParentClass));

    TSharedPtr<AActor> Actor = CreateActorInternal(ActorNode.ParentClass);

    Actor->LoadBlueprint(Asset);
    
    return Actor;
}

void FActorFactory::RegisterActor(const std::string& Name, ActorCreator Creator)
{
    mRegistry[Name] = Creator;
}
