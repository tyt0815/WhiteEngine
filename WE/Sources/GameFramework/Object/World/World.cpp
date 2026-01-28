#include "World.h"
#include "Component/PhysicsComponent.h"
#include "Component/CameraComponent.h"
#include "GUI/GUICore.h"
#include "Pawn/GhostCameraPawn.h"
#include "Physics/PhysicsCore.h"
#include "Physics/Trace.h"
#include "Utility/Timer.h"

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
		ImGui::Text("Tick_PrePhysics:     %.3fms(%.3fms)",			mProfilingData.Time_Tick_PrePhysics,	mMaxProfilingData.Time_Tick_PrePhysics);
		ImGui::Text("Update_Physics:      %.3fms(%.3fms)",			mProfilingData.Time_Update_Physics,		mMaxProfilingData.Time_Update_Physics);
		ImGui::Text("Physics_Event:       %.3fms(%.3fms)",			mProfilingData.Time_Physics_Event,		mMaxProfilingData.Time_Physics_Event);
		ImGui::Text("Tick_PostPhysics:    %.3fms(%.3fms)",			mProfilingData.Time_Tick_PostPhysics,	mMaxProfilingData.Time_Tick_PostPhysics);
		ImGui::Text("Flush_DestroyQueue:  %.3fms(%.3fms)",			mProfilingData.Time_Flush_DestroyQueue,	mMaxProfilingData.Time_Flush_DestroyQueue);
		ImGui::Text("Update_Render_Items: %.3fms(%.3fms)",			mProfilingData.Time_Update_Render_Items,mMaxProfilingData.Time_Update_Render_Items);
		ImGui::Text("Total:               %.3fms(%.3fms)",          mProfilingData.Total(),                 mMaxProfilingData.Total());
	};
	GUI::AddProfilingCommand(Command);
}

void WWorld::Tick(float Delta)
{
	FTimer Timer;

	FProfilingData ProfilingData;

	for (auto TickGroup : GetTickGroups(ETickGroup::ETG_PrePhysics))
	{
		for (WObject* Object : TickGroup)
		{
			Object->Tick(Delta);
		}
	}
	Timer.Tick();
	ProfilingData.Time_Tick_PrePhysics = Timer.GetDeltaMilliSecond();

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
	ProfilingData.Time_Update_Physics = Timer.GetDeltaMilliSecond();

	{
		std::lock_guard<std::mutex> Lock(mEventQueueMutex);
		for (const FPhysicEventInfo& Info : mOnBeginOverlapEventQueue)
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

		for (const FPhysicEventInfo& Info : mOnHitEventQueue)
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
	ProfilingData.Time_Physics_Event = Timer.GetDeltaMilliSecond();

	for (auto TickGroup : GetTickGroups(ETickGroup::ETG_PostPhysics))
	{
		for (WObject* Object : TickGroup)
		{
			Object->Tick(Delta);
		}
	}
	Timer.Tick();
	ProfilingData.Time_Tick_PostPhysics = Timer.GetDeltaMilliSecond();

	FlushDestroyQueue();
	Timer.Tick();
	ProfilingData.Time_Flush_DestroyQueue = Timer.GetDeltaMilliSecond();

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
	ProfilingData.Time_Update_Render_Items = Timer.GetDeltaMilliSecond();

	UpdateProfilingData(Delta, ProfilingData);
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
#if DEBUG_RENDER
	FDebugLine3DVBProxy Proxy;
	Proxy.Start = Start;
	Proxy.End = End;
	Proxy.Color = Color;
	Proxy.LifeSpan = LifeSpan;
	mRenderItemProxy.mDebugLine3DProxies.emplace_back(Proxy);
#endif
}

void WWorld::EnqueueOnBeginOverlapEvent(const FPhysicEventInfo& Info)
{
	std::lock_guard<std::mutex> Lock(mEventQueueMutex);
	mOnBeginOverlapEventQueue.emplace_back(Info);
}

void WWorld::EnqueueOnHitEvent(const FPhysicEventInfo& Info)
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
		Actor->mActiveActorQueueId = -1;
		if (Id < mActiveActorQueue.size() - 1)
		{
			mActiveActorQueue[Id] = mActiveActorQueue.back();
			mActiveActorQueue[Id]->mActiveActorQueueId = Id;

		}
		mActiveActorQueue.pop_back();
		Actor->OnDeactivate();
	}
}

void WWorld::EnqueueTick(WObject* Object)
{
	if (!Object || Object->mTickGroup == ETickGroup::ETG_None || Object->mTickPriority == ETickPriority::ETP_None || Object->mTickId >= 0)
	{
		return;
	}

	std::vector<WObject*>& TickGroup = GetTickGroup(Object->mTickGroup, Object->mTickPriority);
	Object->mTickId = (int)TickGroup.size();
	TickGroup.push_back(Object);
}

void WWorld::DequeueTick(WObject* Object)
{
	if (!Object || Object->mTickGroup == ETickGroup::ETG_None || Object->mTickPriority == ETickPriority::ETP_None || Object->mTickId < 0)
	{
		return;
	}

	int Id = Object->mTickId;
	Object->mTickId = -1;
	std::vector<WObject*>& TickGroup = GetTickGroup(Object->mTickGroup, Object->mTickPriority);
	if (Id < TickGroup.size() - 1)
	{
		TickGroup[Id] = std::move(TickGroup.back());
		TickGroup[Id]->mTickId = Id;
	}
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
	PhysicsComp->mPhysicsCompQueueId = -1;
	if (i < mPhysicsComponentQueue.size() - 1)
	{
		mPhysicsComponentQueue[i] = std::move(mPhysicsComponentQueue.back());
		mPhysicsComponentQueue[i]->mPhysicsCompQueueId = i;
	}
	mPhysicsComponentQueue.pop_back();
}

void WWorld::LineTrace(XMFLOAT3 Start, XMFLOAT3 End, const std::vector<AActor*>& ActorsToIgnore, FHitResult& HitResult, bool bDrawDebug, float DebugDuration)
{
	TArray<JPH::BodyID> BodiesToIgnore;

	ExtractActorsPhysicsBodyID(ActorsToIgnore, BodiesToIgnore);

	LineTrace(Start, End, BodiesToIgnore, HitResult, bDrawDebug, DebugDuration);
}

void WWorld::LineTrace(XMFLOAT3 Start, XMFLOAT3 End, const std::vector<JPH::BodyID>& BodiesToIgnore, FHitResult& HitResult, bool bDrawDebug, float DebugDuration)
{
	Physics::LineTrace(Start, End, HitResult, BodiesToIgnore);

	AfterLineTrace(Start, End, HitResult, bDrawDebug, DebugDuration);
}

void WWorld::LineTraceByObjectChannel(XMFLOAT3 Start, XMFLOAT3 End, const std::vector<AActor*>& ActorsToIgnore, const std::vector<JPH::ObjectLayer> ObjectChannels, FHitResult& HitResult, bool bDrawDebug, float DebugDuration)
{
	TArray<JPH::BodyID> BodiesToIgnore;

	ExtractActorsPhysicsBodyID(ActorsToIgnore, BodiesToIgnore);

	Physics::LineTrace(Start, End, HitResult, ObjectChannels, BodiesToIgnore);

	AfterLineTrace(Start, End, HitResult, bDrawDebug, DebugDuration);
}

void WWorld::SphereOverlap(XMFLOAT3 Location, float Radius, const std::vector<AActor*>& ActorsToIgnore, TArray<FHitResult>& HitResults, bool bDrawDebug, float DebugDuration)
{
	TArray<JPH::BodyID> BodiesToIgnore;
	ExtractActorsPhysicsBodyID(ActorsToIgnore, BodiesToIgnore);
	Physics::SphereOverlap(Location, Radius, HitResults, BodiesToIgnore);
	AfterSphereOverlap(Location, Radius, HitResults, bDrawDebug, DebugDuration);
}

void WWorld::ExtractActorsPhysicsBodyID(const std::vector<AActor*>& Actors, std::vector<JPH::BodyID>& Bodies)
{
	for (const AActor* Actor : Actors)
	{
		for (auto PhysCompWeak : Actor->mAllPhysicsComponents)
		{
			if (auto PhysComp = PhysCompWeak.lock())
			{
				Bodies.push_back(PhysComp->mBody->GetBodyID());
			}
		}
	}
}

void WWorld::AfterLineTrace(XMFLOAT3 Start, XMFLOAT3 End, const FHitResult& HitResult, bool bDrawDebug, float DebugDuration)
{
	if (bDrawDebug)
	{
		XMFLOAT3 DebugStart = Start;
		XMFLOAT3 DebugEnd;
		XMFLOAT4 DebugColor;
		if (HitResult.HitComponent.expired())
		{
			DebugEnd = End;
			DebugColor = { 1,0,0,1 };
		}
		else
		{
			DebugEnd = HitResult.ImpactPoint;
			DebugColor = { 0,1,0,1 };
		}

		DrawDebugLine(DebugStart, DebugEnd, DebugColor, DebugDuration);
	}
}

void WWorld::AfterSphereOverlap(
	XMFLOAT3 Location,
	float Radius,
	const TArray<FHitResult>& HitResults,
	bool bDrawDebug,
	float DebugDuration
)
{
	if (!bDrawDebug) return;

	// 1. 충돌 여부에 따른 구체 가이드라인 색상 (겹치면 녹색, 아니면 빨간색)
	bool bHasHit = (HitResults.size() > 0);
	XMFLOAT4 GuideColor = bHasHit ? XMFLOAT4(0, 1, 0, 1) : XMFLOAT4(1, 0, 0, 1);
	XMFLOAT4 CyanColor = XMFLOAT4(0, 1, 1, 1); // 시안색 (R:0, G:1, B:1)

	// 2. 구체 형태를 나타내는 3축 라인 (Cross)
	// X축
	DrawDebugLine(
		XMFLOAT3(Location.x - Radius, Location.y, Location.z),
		XMFLOAT3(Location.x + Radius, Location.y, Location.z),
		GuideColor, DebugDuration
	);
	// Y축
	DrawDebugLine(
		XMFLOAT3(Location.x, Location.y - Radius, Location.z),
		XMFLOAT3(Location.x, Location.y + Radius, Location.z),
		GuideColor, DebugDuration
	);
	// Z축
	DrawDebugLine(
		XMFLOAT3(Location.x, Location.y, Location.z - Radius),
		XMFLOAT3(Location.x, Location.y, Location.z + Radius),
		GuideColor, DebugDuration
	);

	// 3. 중심점에서 각 충돌 지점까지 라인 그리기
	for (const FHitResult& Hit : HitResults)
	{
		// 구체 중심(Location) -> 충돌 지점(ImpactPoint) 연결
		DrawDebugLine(
			Location,
			Hit.ImpactPoint,
			CyanColor,
			DebugDuration
		);

		// (선택사항) 충돌 지점 끝에 아주 작은 점 하나 찍어주면 더 잘 보입니다.
		float TipSize = 0.02f;
		DrawDebugLine(
			XMFLOAT3(Hit.ImpactPoint.x - TipSize, Hit.ImpactPoint.y, Hit.ImpactPoint.z),
			XMFLOAT3(Hit.ImpactPoint.x + TipSize, Hit.ImpactPoint.y, Hit.ImpactPoint.z),
			CyanColor, DebugDuration
		);
	}
}

void WWorld::FlushDestroyQueue()
{
	auto Copy = DestroyQueue;
	DestroyQueue.clear();
	for (int i = 0; i < Copy.size(); ++i)
	{
		TSharedPtr<AActor>& Actor = Copy[i];
		int Id = Actor->mActorId;
		Actor->mActorId = -1;
		if (Id < mAllActors.size() - 1)
		{
			mAllActors[Id] = std::move(mAllActors.back());
			mAllActors[Id]->mActorId = Id;
		}
		mAllActors.pop_back();
		Actor->OnDestroy();
	}
}

void WWorld::UpdateProfilingData(float DeltaSecond, const FProfilingData& Data)
{
	mMaxProfilingData.Time_Tick_PrePhysics =	max(mMaxProfilingData.Time_Tick_PrePhysics, Data.Time_Tick_PrePhysics);
	mMaxProfilingData.Time_Update_Physics =		max(mMaxProfilingData.Time_Update_Physics, Data.Time_Update_Physics);
	mMaxProfilingData.Time_Physics_Event =		max(mMaxProfilingData.Time_Physics_Event, Data.Time_Physics_Event);
	mMaxProfilingData.Time_Tick_PostPhysics =	max(mMaxProfilingData.Time_Tick_PostPhysics, Data.Time_Tick_PostPhysics);
	mMaxProfilingData.Time_Flush_DestroyQueue = max(mMaxProfilingData.Time_Flush_DestroyQueue, Data.Time_Flush_DestroyQueue);
	mMaxProfilingData.Time_Update_Render_Items = max(mMaxProfilingData.Time_Update_Render_Items, Data.Time_Update_Render_Items);

	static float ElapsedTime = 0;
	static int CallCount = 0;
	static FProfilingData AccumulatedData;
	ElapsedTime += DeltaSecond;
	++CallCount;

	AccumulatedData.Time_Tick_PrePhysics		+= Data.Time_Tick_PrePhysics;
	AccumulatedData.Time_Update_Physics			+= Data.Time_Update_Physics;
	AccumulatedData.Time_Physics_Event			+= Data.Time_Physics_Event;
	AccumulatedData.Time_Tick_PostPhysics		+= Data.Time_Tick_PostPhysics;
	AccumulatedData.Time_Flush_DestroyQueue		+= Data.Time_Flush_DestroyQueue;
	AccumulatedData.Time_Update_Render_Items	+= Data.Time_Update_Render_Items;

	if (ElapsedTime > 1)
	{
		mProfilingData.Time_Tick_PrePhysics = AccumulatedData.Time_Tick_PrePhysics / CallCount;
		mProfilingData.Time_Update_Physics = AccumulatedData.Time_Update_Physics / CallCount;
		mProfilingData.Time_Physics_Event = AccumulatedData.Time_Physics_Event / CallCount;
		mProfilingData.Time_Tick_PostPhysics = AccumulatedData.Time_Tick_PostPhysics / CallCount;
		mProfilingData.Time_Flush_DestroyQueue = AccumulatedData.Time_Flush_DestroyQueue / CallCount;
		mProfilingData.Time_Update_Render_Items = AccumulatedData.Time_Update_Render_Items / CallCount;

		ElapsedTime = 0;
		CallCount = 0;
		ZeroMemory(&AccumulatedData, sizeof(FProfilingData));
	}
}		
		
		