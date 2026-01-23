#include "World.h"
#include "Physics/PhysicsCore.h"
#include "../Pawn/GhostCameraPawn.h"
#include "Component/PhysicsComponent.h"
#include "GUI/GUICore.h"
#include "Utility/Timer.h"
#include "Component/CameraComponent.h"

WWorld* g_World;

WWorld::WWorld()
{
	g_World = this;
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

	// 프로파일링용 GUI
	GUI::FDrawCommand Command;
	Command.LifeSpan = -1;
	Command.DrawLambda = [&]()
	{
		ImGui::TextColored(ImVec4(1, 1, 0, 1), "WWorld::Tick"); // 노란색 제목
		ImGui::Separator();
		ImGui::Text("Tick_PrePhysics: %f", mTime_Tick_PrePhysics);
		ImGui::Text("Update_Physics: %f", mTime_Update_Physics);
		ImGui::Text("Physics_Event: %f", mTime_Physics_Event);
		ImGui::Text("Tick_PostPhysics: %f", mTime_Tick_PostPhysics);
		ImGui::Text("Update_Render_Items: %f", mTime_Update_Render_Items);
	};
	GUI::AddProfilingCommand(Command);
}

void WWorld::Tick(float Delta)
{
	FTimer Timer;

	for (AActor* Actor : mActorTickGroups[ETickGroup::ETG_PrePhysics])
	{
		Actor->Tick(Delta);
	}
	for (WActorComponent* Comp : mActorComponentTickGroups[ETickGroup::ETG_PrePhysics])
	{
		Comp->TickComponent(Delta);
	}
	Timer.Tick();
	mTime_Tick_PrePhysics = Timer.GetDeltaTime();

	for (WPhysicsComponent* Comp : mPhysicsComponentQueue)
	{
		Comp->UpdateToPhysics();
	}
	Physics::Tick(Delta);
	for (WPhysicsComponent* Comp : mPhysicsComponentQueue)
	{
		Comp->UpdateFromPhysics();
	}
	Timer.Tick();
	mTime_Update_Physics = Timer.GetDeltaTime();

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
	Timer.Tick();
	mTime_Physics_Event = Timer.GetDeltaTime();

	for (AActor* Actor : mActorTickGroups[ETickGroup::ETG_PostPhysics])
	{
		Actor->Tick(Delta);
	}
	for (WActorComponent* Comp : mActorComponentTickGroups[ETickGroup::ETG_PostPhysics])
	{
		Comp->TickComponent(Delta);
	}
	Timer.Tick();
	mTime_Tick_PostPhysics = Timer.GetDeltaTime();

	FlushDestroyQueue();

	// RenderItem 업데이트
	
	mRenderItemProxyMetex.lock();
	mRenderItemProxy.Cleanup(Delta);
	for (auto& Actor : mActiveActorQueue)
	{
		Actor->UpdateRecursive();
	}

	if (auto Camera = mPlayer.lock()->GetCameraComponent().lock())
	{
		mRenderItemProxy.ViewMatrix = Camera->GetViewMatrix();
		mRenderItemProxy.ProjMatrix = Camera->GetProjMatrix();
		mRenderItemProxy.EyePosW = Camera->GetWorldLocation();
		mRenderItemProxy.NearZ = Camera->GetNearZ();
		mRenderItemProxy.FarZ = Camera->GetFarZ();
	}
	mRenderItemProxyMetex.unlock();
	
	Timer.Tick();
	mTime_Update_Render_Items = Timer.GetDeltaTime();
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
	if (Actor && !Actor->IsPendingKill())
	{
		Actor->mbPendingKill = true;
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

void WWorld::ActivateActor(AActor* Actor)
{
	if (!Actor->IsActivated())
	{
		mActiveActorQueue.emplace_back(Actor);
		Actor->mActiveActorQueueId = (int)mActiveActorQueue.size() - 1;
		Actor->OnActivate();
	}
}

void WWorld::DeactivateActor(AActor* Actor)
{
	if (Actor->IsActivated())
	{
		int Id = Actor->mActiveActorQueueId;
		mActiveActorQueue[Id] = std::move(mActiveActorQueue.back());
		mActiveActorQueue[Id]->mActiveActorQueueId = Id;
		mActiveActorQueue.pop_back();
		Actor->mActiveActorQueueId = -1;
		Actor->OnDeactivate();
	}
}

void WWorld::EnqueueComponentTick(WActorComponent* ActorComp)
{
	if (!ActorComp || ActorComp->mTickGroup == ETickGroup::ETG_None || ActorComp->mTickQueueId >= 0)
	{
		return;
	}

	std::vector<WActorComponent*>& TickGroup = mActorComponentTickGroups[ActorComp->mTickGroup];
	ActorComp->mTickQueueId = (int)TickGroup.size();
	TickGroup.push_back(ActorComp);
}

void WWorld::EnqueueActorTick(AActor* Actor)
{
	if (!Actor || Actor->mTickGroup == ETickGroup::ETG_None || Actor->mTickQueueId >= 0)
	{
		return;
	}

	std::vector<AActor*>& TickGroup = mActorTickGroups[Actor->mTickGroup];
	Actor->mTickQueueId = (int)TickGroup.size();
	TickGroup.push_back(Actor);
}

void WWorld::DequeueComponentTick(WActorComponent* ActorComp)
{
	if (!ActorComp || ActorComp->mTickGroup == ETickGroup::ETG_None || ActorComp->mTickQueueId < 0)
	{
		return;
	}

	int i = ActorComp->mTickQueueId;
	std::vector<WActorComponent*>& TickGroup = mActorComponentTickGroups[ActorComp->mTickGroup];
	TickGroup[i] = std::move(TickGroup.back());
	TickGroup[i]->mTickQueueId = i;
	TickGroup.pop_back();
}

void WWorld::DequeueActorTick(AActor* Actor)
{
	if (!Actor || Actor->mTickGroup == ETickGroup::ETG_None || Actor->mTickQueueId < 0)
	{
		return;
	}

	int i = Actor->mTickQueueId;
	std::vector<AActor*>& TickGroup = mActorTickGroups[Actor->mTickGroup];
	TickGroup[i] = std::move(TickGroup.back());
	TickGroup[i]->mTickQueueId = i;
	TickGroup.pop_back();
}

void WWorld::EnqueuePhysicsComponent(WPhysicsComponent* PhysicsComp)
{
	if (!PhysicsComp || PhysicsComp->mPhysicsCompQueueId >= 0)
	{
		return;
	}

	PhysicsComp->mPhysicsCompQueueId = (int)mPhysicsComponentQueue.size();
	mPhysicsComponentQueue.push_back(PhysicsComp);
}

void WWorld::DequeuePhysicsComponent(WPhysicsComponent* PhysicsComp)
{
	if (!PhysicsComp || PhysicsComp->mPhysicsCompQueueId < 0)
	{
		return;
	}

	int i = PhysicsComp->mPhysicsCompQueueId;
	mPhysicsComponentQueue[i] = std::move(mPhysicsComponentQueue.back());
	mPhysicsComponentQueue[i]->mPhysicsCompQueueId = i;
	mPhysicsComponentQueue.pop_back();
}

void WWorld::FlushDestroyQueue()
{
	for (auto Actor : DestroyQueue)
	{
		Actor->OnDestroy();
		int Id = Actor->mActorId;
		mAllActors[Id] = std::move(mAllActors.back());
		mAllActors[Id]->mActorId = Id;
		mAllActors.pop_back();
	}
	DestroyQueue.clear();
}
