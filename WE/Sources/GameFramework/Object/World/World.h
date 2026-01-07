#pragma once

#include "GameFramework/Object/Pawn/Pawn.h"
#include "Utility/Container.h"
#include "Utility/Class.h"

class WWorld
{
public:
	WWorld();

	virtual ~WWorld();

public:
	virtual void Tick(float Delta);

	void SetPlayer(APawn* Player);

	template<typename T>
	T* SpawnActor();

private:
	TUnorderedArray<std::unique_ptr<AActor>> mAllActors;
	APawn* mPlayer = nullptr;

public:
	inline APawn* GetPlayer() const
	{
		return mPlayer;
	}

	inline WCameraComponent* GetPlayerCamera() const
	{
		return mPlayer->GetCameraComponent();
	}
};

extern WWorld* gWorld;

inline WWorld* GetWorld()
{
	return gWorld;
}

template<typename T>
inline T* WWorld::SpawnActor()
{
	size_t Index = mAllActors.Add(std::make_unique<T>());
	T* Actor = dynamic_cast<T*>(mAllActors[Index].get());

	if (Actor->GetRootComponent() == nullptr)
	{
		WSceneComponent* DummyRoot = Actor->CreateComponent<WSceneComponent>();
		Actor->SetRootComponent(DummyRoot);
	}

	return Actor;
}
