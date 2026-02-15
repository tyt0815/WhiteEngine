#include "StateMachineActor.h"
#include "Component/StateMachineComponent.h"
#include "Component/ProjectileMovementComponent.h"
#include "Component/ObjectAnimComponent.h"
#include "GameFramework/Interface/CollisionGenerator.h"

AStateMachineActor::AStateMachineActor()
{
	mStateMachine = CreateComponent<WStateMachineComponent>();
	AddWComponent("StateMachine", mStateMachine);

	SetTickGroup(ETickGroup::ETG_PrePhysics, ETickPriority::ETP_Low);
}

void AStateMachineActor::BeginPlay()
{
	Super::BeginPlay();

	mStateMachine->SendEvent("OnSpawn");
}

void AStateMachineActor::Tick(float DeltaSecond)
{
	Super::Tick(DeltaSecond);
	
	mStateMachine->SendEvent("OnUpdate");
}

void AStateMachineActor::LoadBlueprint(const FBlueprintAsset* Asset)
{
	Super::LoadBlueprint(Asset);

	const auto& SMData = Asset->mRuntimeStateMachine;
	if (SMData.bExist)
	{
		for (const auto& [StateName, StateSetup] : SMData.States)
		{
			// 스테이트 인스턴스 생성 및 부모 설정(상속 처리)
			WState* NewState = mStateMachine->CreateState(StateName, StateSetup.BaseName);

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

		

		for (const auto& Setup : Asset->mComponentSetups)
		{
			if (WActorComponent* Comp = GetWComponent<WActorComponent>(Setup.Name))
			{
				const std::string& CompName = Setup.Name;

				auto BindSMEvent = [this, &CompName](auto& Delegate, const std::string& Prefix)
				{
					std::string FullTag = Prefix + "_" + CompName;

					Delegate.AddLambda([this, FullTag](auto&&...)
						{
							mStateMachine->SendEvent(FullTag);
						});
				};

				BindSMEvent(Comp->mOnActivate, "OnActivate");
				BindSMEvent(Comp->mOnDeactivate, "OnDeactivate");

				if (FCollisionGeneratorBase* CollisionComp = dynamic_cast<FCollisionGeneratorBase*>(Comp))
				{
					BindSMEvent(CollisionComp->mOnCollision, "OnCollision");
				}
				else if (WProjectileMovementComponent* ProjComp = dynamic_cast<WProjectileMovementComponent*>(Comp))
				{
					BindSMEvent(ProjComp->mOnLockon, "OnLockon");
					BindSMEvent(ProjComp->mOnBounce, "OnBounce");
					BindSMEvent(ProjComp->mOnHomingFail, "OnHomingFail");
				}
				else if (WObjectAnimComponent* AnimComp = dynamic_cast<WObjectAnimComponent*>(Comp))
				{
					BindSMEvent(AnimComp->mOnStop, "OnAnimStop");
				}
			}
		}

		// 초기 상태 설정
		mStateMachine->SetInitialState(SMData.InitialState);
	}

	
}

void AStateMachineActor::OnDestroy()
{
	mStateMachine->SendEvent("OnDestory");
	Super::OnDestroy();
}
