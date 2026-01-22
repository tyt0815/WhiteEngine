#include "World.h"
#include "Physics/PhysicsCore.h"
#include "../Pawn/GhostCameraPawn.h"
#include "Component/PhysicsComponent.h"
#include "GUI/GUICore.h"
#include "Utility/Timer.h"
#include "Component/CameraComponent.h"

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
	UTimer Timer;
	for (auto& Actor : mActiveActorQueue)
	{
		// Tick Component 가 포함되어 있음
		Actor->Tick_PrePhysics(Delta);
	}
	Timer.Tick();
	double Time_Tick_PrePhysics = Timer.GetDeltaTime();

	for (auto& Actor : mActiveActorQueue)
	{
		Actor->UpdateComponentsToPhysics();
	}
	Physics::Tick(Delta);
	for (auto& Actor : mActiveActorQueue)
	{
		Actor->UpdateComponentsFromPhysics();
	}
	Timer.Tick();
	double Time_Update_Physics = Timer.GetDeltaTime();

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
	double Time_Physics_Event = Timer.GetDeltaTime();

	for (auto& Actor : mActiveActorQueue)
	{
		// Tick Component 가 포함되어 있음
		Actor->Tick_PostPhysics(Delta);
	}
	Timer.Tick();
	double Time_Tick_PostPhysics = Timer.GetDeltaTime();

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
	double Time_Update_Render_Items = Timer.GetDeltaTime();

	// 프로파일링용 GUI
	GUI::FDrawCommand Command;
	Command.LifeSpan = 0;
	Command.DrawLambda = [=]()
	{
		ImGui::TextColored(ImVec4(1, 1, 0, 1), "WWorld::Tick"); // 노란색 제목
		ImGui::Separator();
		ImGui::Text("Tick_PrePhysics: %f", Time_Tick_PrePhysics);
		ImGui::Text("Update_Physics: %f", Time_Update_Physics);
		ImGui::Text("Physics_Event: %f", Time_Physics_Event);
		ImGui::Text("Tick_PostPhysics: %f", Time_Tick_PostPhysics);
		ImGui::Text("Update_Render_Items: %f", Time_Update_Render_Items);
	};
	// GUI::AddProfilingCommand(Command);
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
		Actor->mTickQueueId = mActiveActorQueue.size() - 1;
		Actor->OnActivate();
	}
}

void WWorld::DeactivateActor(AActor* Actor)
{
	if (Actor->IsActivated())
	{
		INT64 Id = Actor->mTickQueueId;
		mActiveActorQueue[Id] = std::move(mActiveActorQueue.back());
		mActiveActorQueue[Id]->mTickQueueId = Id;
		mActiveActorQueue.pop_back();
		Actor->mTickQueueId = -1;
		Actor->OnDeactivate();
	}
}

void WWorld::FlushDestroyQueue()
{
	bool bLoop = false;
	float Time = 0;
	UTimer Timer;
	for (auto Actor : DestroyQueue)
	{
		bLoop = true;
		DeactivateActor(Actor.get());
		UINT64 Id = Actor->mActorId;
		mAllActors[Id] = std::move(mAllActors.back());
		mAllActors[Id]->mActorId = Id;
		mAllActors.pop_back();
	}
	DestroyQueue.clear();

	Timer.Tick();
	if (bLoop)
	{
		double Time = Timer.GetDeltaTime();

		GUI::FDrawCommand Command;
		Command.LifeSpan = 2;
		Command.DrawLambda = [=]()
		{
			ImGui::SetNextWindowPos(ImVec2(1300, 0), ImGuiCond_Always);
			ImGui::SetNextWindowSize(ImVec2(300, 0));

			ImGuiWindowFlags WindowFlags = ImGuiWindowFlags_NoDecoration |
				ImGuiWindowFlags_AlwaysAutoResize |
				ImGuiWindowFlags_NoSavedSettings |
				ImGuiWindowFlags_NoFocusOnAppearing |
				ImGuiWindowFlags_NoNav |
				ImGuiWindowFlags_NoMove;

			ImGui::SetNextWindowBgAlpha(1.0f);

			if (ImGui::Begin("Destroying", nullptr, WindowFlags))
			{

				ImGui::TextColored(ImVec4(0, 1, 1, 1), "Destroying Time");
				ImGui::Separator();

				ImGui::Text("%.8f", Time);
			}
			ImGui::End();
		};

		GUI::AddDrawCommand(Command);
	}

}
