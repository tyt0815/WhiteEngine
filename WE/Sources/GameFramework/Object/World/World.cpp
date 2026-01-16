#include "World.h"
#include "Physics/PhysicsCore.h"
#include "../Pawn/GhostCameraPawn.h"
#include "Component/PhysicsComponent.h"

WWorld* gWorld;

WWorld::WWorld()
{
	gWorld = this;
}

WWorld::~WWorld()
{
}

void WWorld::BeginPlay()
{
	if (mPlayer == nullptr)
	{
		APawn* Player = SpawnActor<AGhostCameraPawn>();
		SetPlayer(Player);
	}
}

void WWorld::Tick(float Delta)
{
	mRenderItemProxy.Cleanup(Delta);

	for (size_t i = 0; i < mAllActors.Size(); ++i)
	{
		// Tick Component 가 포함되어 있음
		mAllActors[i]->Tick_PrePhysics(Delta);
	}

	for (auto& Actor : mAllActors.GetView())
	{
		Actor->UpdateComponentsToPhysics();
	}
	Physics::Tick(Delta);
	for (auto& Actor : mAllActors.GetView())
	{
		Actor->UpdateComponentsFromPhysics();
	}
	for (const FContactInfo& Info : mOnBeginOverlapEventQueue)
	{
		Info.Comp1->mOnBeginOverlapDelegate.Execute(Info.Comp2, Info.ImpactPoint1);
		Info.Comp2->mOnBeginOverlapDelegate.Execute(Info.Comp1, Info.ImpactPoint2);
	}
	mOnBeginOverlapEventQueue.clear();
	for (const FContactInfo& Info : mOnHitEventQueue)
	{
		Info.Comp1->mOnHitDelegate.Execute(Info.Comp2, Info.ImpactPoint1);
		Info.Comp2->mOnHitDelegate.Execute(Info.Comp1, Info.ImpactPoint2);
	}
	mOnHitEventQueue.clear();

	for (size_t i = 0; i < mAllActors.Size(); ++i)
	{
		// Tick Component 가 포함되어 있음
		mAllActors[i]->Tick_PostPhysics(Delta);
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

void WWorld::DrawDebugLine(XMFLOAT3 Start, XMFLOAT3 End, XMFLOAT4 Color, float LifeSpan)
{
	FDebugLine3DVBProxy Proxy;
	Proxy.Start = Start;
	Proxy.End = End;
	Proxy.Color = Color;
	Proxy.LifeSpan = LifeSpan;
	mRenderItemProxy.mDebugLine3DProxies.Add(Proxy);
}

// TODO: 같은 주소에 Actor 메모리가 할당되면 당글링 포인터도 true를 반환 할 수 있음
bool WWorld::IsValidActor(AActor* Actor)
{
	for (int i = 0; i < mAllActors.Size(); ++i)
	{
		if (mAllActors[i].get() == Actor)
		{
			return true;
		}
	}
	return false;
}

void WWorld::EnqueueOnBeginOverlapEvent(const FContactInfo& Info)
{
	mOnBeginOverlapEventQueue.emplace_back(Info);
}

void WWorld::EnqueueOnHitEvent(const FContactInfo& Info)
{
	mOnHitEventQueue.emplace_back(Info);
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
