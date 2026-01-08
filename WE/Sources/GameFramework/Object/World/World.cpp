#include "World.h"

WWorld* gWorld;

WWorld::WWorld()
{
	gWorld = this;
}

WWorld::~WWorld()
{
}

void WWorld::Tick(float Delta)
{
	mRenderItemProxy.Cleanup();

	for (size_t i = 0; i < mAllActors.Size(); ++i)
	{
		mAllActors[i]->Tick(Delta);
	}

	for (size_t i = 0; i < mAllActors.Size(); ++i)
	{
		auto& Components = mAllActors[i]->GetAllComponents();
		for (size_t j = 0; j < Components.Size(); ++j)
		{
			Components[j]->TickComponent(Delta);
		}
	}

	for (size_t i = 0; i < mAllActors.Size(); ++i)
	{
		mAllActors[i]->GetRootComponent()->UpdateRecursive();
	}
}

void WWorld::SetPlayer(APawn* Player)
{
	mPlayer = Player;
	mPlayer->SetupPlayerInput();
}