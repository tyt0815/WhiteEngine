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
	if (mPlayer.expired())
	{
		TWeakPtr<APawn> Player = SpawnActor<AGhostCameraPawn>();
		SetPlayer(Player);
	}
}

void WWorld::Tick(float Delta)
{
	mRenderItemProxy.Cleanup(Delta);

	for (size_t i = 0; i < mAllActors.size(); ++i)
	{
		// Tick Component 가 포함되어 있음
		mAllActors[i]->Tick_PrePhysics(Delta);
	}

	for (auto& Actor : mAllActors)
	{
		Actor->UpdateComponentsToPhysics();
	}
	Physics::Tick(Delta);
	for (auto& Actor : mAllActors)
	{
		Actor->UpdateComponentsFromPhysics();
	}
	{
		std::lock_guard<std::mutex> Lock(mEventQueueMutex);
		for (const FContactInfo& Info : mOnBeginOverlapEventQueue)
		{
			if (auto Comp1 = Info.Comp1.lock())
			{
				if (auto Comp2 = Info.Comp2.lock())
				{
					Comp1->mOnBeginOverlapDelegate.Execute(Info.Comp2, Info.ImpactPoint1);
					Comp2->mOnBeginOverlapDelegate.Execute(Info.Comp1, Info.ImpactPoint2);
				}
			}
		}
		mOnBeginOverlapEventQueue.clear();

		for (const FContactInfo& Info : mOnHitEventQueue)
		{
			if (auto Comp1 = Info.Comp1.lock())
			{
				if (auto Comp2 = Info.Comp2.lock())
				{
					Comp1->mOnHitDelegate.Execute(Info.Comp2, Info.ImpactPoint1);
					Comp2->mOnHitDelegate.Execute(Info.Comp1, Info.ImpactPoint2);
				}
			}
		}
		mOnHitEventQueue.clear();
	}
	

	for (size_t i = 0; i < mAllActors.size(); ++i)
	{
		// Tick Component 가 포함되어 있음
		mAllActors[i]->Tick_PostPhysics(Delta);
	}

	FlushDestroyQueue();

	for (size_t i = 0; i < mAllActors.size(); ++i)
	{
		if (auto Root = mAllActors[i]->GetRootComponent().lock())
		{
			Root->UpdateRecursive();
		}
		
	}
}

TWeakPtr<WCameraComponent> WWorld::GetPlayerCamera() const
{
	if (TSharedPtr<APawn> Player = mPlayer.lock())
	{
		return Player->GetCameraComponent();
	}
	return TWeakPtr<WCameraComponent>();
}

void WWorld::SetPlayer(TWeakPtr<APawn> Player)
{
	mPlayer = Player;
	if (TSharedPtr<APawn> Pawn = mPlayer.lock())
	{
		Pawn->SetupPlayerInput();
	}
}

void WWorld::DestroyActor(const TSharedPtr<AActor>& Actor)
{
	if(Actor && !Actor->IsPendingKill())
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
	mRenderItemProxy.mDebugLine3DProxies.emplace_back(Proxy);
}

void WWorld::EnqueueOnBeginOverlapEvent(const FContactInfo& Info)
{
	std::lock_guard<std::mutex> Lock(mEventQueueMutex);
	mOnBeginOverlapEventQueue.emplace_back(Info);
}

void WWorld::EnqueueOnHitEvent(const FContactInfo& Info)
{
	std::lock_guard<std::mutex> Lock(mEventQueueMutex);
	mOnHitEventQueue.emplace_back(Info);
}

void WWorld::FlushDestroyQueue()
{
	for (auto Actor : DestroyQueue)
	{
		UINT64 Id = Actor->GetActorId();
		mAllActors[Id] = std::move(mAllActors.back());
		mAllActors[Id]->SetActorId(Id);
		mAllActors.pop_back();
	}

	DestroyQueue.clear();
}
