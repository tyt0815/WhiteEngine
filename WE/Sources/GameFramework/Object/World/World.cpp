#include "World.h"
#include "Physics/PhysicsCore.h"

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
	mRenderItemProxy.Cleanup(Delta);

	for (size_t i = 0; i < mAllActors.Size(); ++i)
	{
		mAllActors[i]->Tick_PrePhysics(Delta);
	}

	for (size_t i = 0; i < mAllActors.Size(); ++i)
	{
		auto& Components = mAllActors[i]->GetAllComponents();
		for (size_t j = 0; j < Components.Size(); ++j)
		{
			Components[j]->TickComponent_PrePhysics(Delta);
		}
	}

	Physics::Tick(Delta);
	for (size_t i = 0; i < mAllActors.Size(); ++i)
	{
		mAllActors[i]->UpdatePhysics();
	}

	for (size_t i = 0; i < mAllActors.Size(); ++i)
	{
		mAllActors[i]->Tick_PostPhysics(Delta);
	}

	for (size_t i = 0; i < mAllActors.Size(); ++i)
	{
		auto& Components = mAllActors[i]->GetAllComponents();
		for (size_t j = 0; j < Components.Size(); ++j)
		{
			Components[j]->TickComponent_PostPhysics(Delta);
		}
	}

	FlushDestroyQueue();

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

void WWorld::DestroyActor(AActor* Actor)
{
	if (Actor->IsValid())
	{
		Actor->MarkPendingKill();
		DestroyQueue.push_back(Actor);
	}
}

void WWorld::DrawDebugLine(XMFLOAT3 Location, XMFLOAT4 Color, float LifeSpan)
{
	FDebugLine3DVBProxy Proxy;
	Proxy.Position = Location;
	Proxy.Color = Color;
	Proxy.LifeSpan = LifeSpan;
	mRenderItemProxy.mDebugLine3DProxies.Add(Proxy);
}

void WWorld::FlushDestroyQueue()
{
	for (AActor* Actor : DestroyQueue)
	{
		size_t Id = Actor->GetActorId();
		mAllActors.RemoveAt(Id);
		if (Id < mAllActors.Size())
		{
			mAllActors[Id]->SetActorId(Id);
		}
	}

	DestroyQueue.clear();
}
