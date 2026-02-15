#include "StateMachineActor.h"
#include "Component/StateMachineComponent.h"

AStateMachineActor::AStateMachineActor()
{
	WStateMachineComponent* Comp = CreateComponent<WStateMachineComponent>();
	AddWComponent("StateMachine", Comp);
	mStateMachineComp = Comp->GetWeakPtr<WStateMachineComponent>();

	SetTickGroup(ETickGroup::ETG_PrePhysics, ETickPriority::ETP_Low);
}

void AStateMachineActor::BeginPlay()
{
	Super::BeginPlay();

	mStateMachineComp.lock()->SendEvent("OnSpawn");
}

void AStateMachineActor::Tick(float DeltaSecond)
{
	Super::Tick(DeltaSecond);

	auto StateMachine = mStateMachineComp.lock();
	
	StateMachine->SendEvent("OnUpdate");
}

void AStateMachineActor::LoadBlueprint(const FBlueprintAsset* Asset)
{
	Super::LoadBlueprint(Asset);

	const auto& SMData = Asset->mRuntimeStateMachine;
	if (SMData.bExist)
	{
		// 액터 전용 스테이트 머신 인스턴스 생성
		TSharedPtr<WStateMachineComponent> StateMachine = mStateMachineComp.lock(); // 액터가 보유한 SM 컴포넌트 획득

		for (const auto& [StateName, StateSetup] : SMData.States)
		{
			// 스테이트 인스턴스 생성 및 부모 설정(상속 처리)
			WState* NewState = StateMachine->CreateState(StateName, StateSetup.BaseName);

			for (const auto& Binding : StateSetup.EventBindings)
			{
				// 스테이트 내부 이벤트 생성 및 액션 바인딩
				WEvent* StateEvent = NewState->GetOrCreateEvent(Binding.Tag, Binding.Attributes);
				for (const auto& ActionFactory : Binding.ActionFactories)
				{
					StateEvent->AddAction(ActionFactory(this));
				}
			}
		}

		// 초기 상태 설정
		StateMachine->SetInitialState(SMData.InitialState);
	}

	//for (const auto& Binding : SMData.CustomEvents)
	//{
	//	WEventRegistry::GetInstance()->Register(Binding.Tag, [](WObject* Target, const WAttributesMap& Attr) -> WEvent* {
	//		AActor* Owner = static_cast<AActor*>(Target);
	//		return Owner->GetOnSpawnEvent(); // 액터가 기본으로 가진 mOnSpawnEvent 반환
	//		});
	//	WEvent* CustomEvent = WEventRegistry::GetInstance()->Create(Binding.Tag, this, Binding.Attributes);
	//	for (const auto& ActionFactory : Binding.ActionFactories)
	//	{
	//		CustomEvent->AddAction(ActionFactory(this));
	//	}
	//}
}
