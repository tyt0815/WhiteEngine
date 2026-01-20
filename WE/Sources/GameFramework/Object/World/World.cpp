#include "World.h"
#include "Physics/PhysicsCore.h"
#include "../Pawn/GhostCameraPawn.h"
#include "Component/PhysicsComponent.h"
#include "GUI/GUICore.h"
#include "Utility/Timer.h"

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

	UTimer Timer;
	for (size_t i = 0; i < mAllActors.size(); ++i)
	{
		// Tick Component 가 포함되어 있음
		mAllActors[i]->Tick_PrePhysics(Delta);
	}
	Timer.Tick();
	float Time_Tick_PrePhysics = Timer.GetDeltaTime();

	for (auto& Actor : mAllActors)
	{
		Actor->UpdateComponentsToPhysics();
	}
	Physics::Tick(Delta);
	for (auto& Actor : mAllActors)
	{
		Actor->UpdateComponentsFromPhysics();
	}
	Timer.Tick();
	float Time_Update_Physics = Timer.GetDeltaTime();

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
	float Time_Physics_Event = Timer.GetDeltaTime();


	for (size_t i = 0; i < mAllActors.size(); ++i)
	{
		// Tick Component 가 포함되어 있음
		mAllActors[i]->Tick_PostPhysics(Delta);
	}
	Timer.Tick();
	float Time_Tick_PostPhysics = Timer.GetDeltaTime();

	FlushDestroyQueue();

	for (size_t i = 0; i < mAllActors.size(); ++i)
	{
		if (auto Root = mAllActors[i]->GetRootComponent().lock())
		{
			Root->UpdateRecursive();
		}
	}
	Timer.Tick();
	float Time_Update_Render_Items = Timer.GetDeltaTime();



	// 프로파일링용 GUI
	GUI::FDrawCommand Command;
	Command.LifeSpan = 0;
	Command.DrawLambda = [=]()
	{
		ImGui::SetNextWindowPos(ImVec2(0, 50), ImGuiCond_Always);

		ImGuiWindowFlags WindowFlags = ImGuiWindowFlags_NoDecoration |
			ImGuiWindowFlags_AlwaysAutoResize |
			ImGuiWindowFlags_NoSavedSettings |
			ImGuiWindowFlags_NoFocusOnAppearing |
			ImGuiWindowFlags_NoNav |
			ImGuiWindowFlags_NoMove;

		ImGui::SetNextWindowBgAlpha(1.0f);

		if (ImGui::Begin("World Tick", nullptr, WindowFlags))
		{
			ImGui::TextColored(ImVec4(1, 1, 0, 1), "World Tick Profiling"); // 노란색 제목
			ImGui::Separator();

			ImGui::Text("Tick_PrePhysics: %f", Time_Tick_PrePhysics);
			ImGui::Text("Update_Physics: %f", Time_Update_Physics);
			ImGui::Text("Physics_Event: %f", Time_Physics_Event);
			ImGui::Text("Tick_PostPhysics: %f", Time_Tick_PostPhysics);
			ImGui::Text("Update_Render_Items: %f", Time_Update_Render_Items);
		}
		ImGui::End();
	};
	GUI::AddDrawCommand(Command);
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
