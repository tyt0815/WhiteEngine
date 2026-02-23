#include "StateMachineActor.h"
#include "Component/StateMachineComponent.h"
#include "Component/ProjectileMovementComponent.h"
#include "Component/ObjectAnimComponent.h"
#include "Component/SplineComponent.h"
#include "GameFramework/Interface/CollisionGenerator.h"
#include "GameFramework/Interface/HitInterface.h"
#include "Parser.h"

AStateMachineActor::AStateMachineActor()
{
	mStateMachine = CreateComponent<WStateMachineComponent>();
	RegisterWObject("StateMachine", mStateMachine);
	
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
			WState* NewState = mStateMachine->CreateStateOrGet(StateName, StateSetup.BaseName);

			for (const auto& Binding : StateSetup.EventBindings)
			{
				// 스테이트 내부 이벤트 생성 및 액션 바인딩
				WEvent* StateEvent = NewState->GetOrCreateEvent(Binding.Tag, Binding.Attributes);
				for (const auto& ActionFactory : Binding.ActionFactories)
				{
					StateEvent->AddAction(ActionFactory(this));
				}
			}

			for (const auto& TBinding : StateSetup.TransitionBindings)
			{
				// Attributes에서 "Event" 태그가 있는지 확인 (사용자님의 명명 규칙에 따라 "Event" 키 사용)
				auto it = TBinding.Attributes.find("Event");
				std::string EventName = (it != TBinding.Attributes.end()) ? it->second : "";

				// Condition 수식 바인딩 (Condition 속성이 있다면 람다 생성)
				std::function<bool()> CondFunc = nullptr;
				if (TBinding.Attributes.count("Condition"))
				{
					// WExpressionParser 등을 통해 bool을 반환하는 람다 생성
					CondFunc = WExpressionParser::Bind<bool>(this, TBinding.Attributes, "Condition", "false");
				}

				if (!EventName.empty())
				{
					// Case 1 & 2: Event 기반 전이 (Target + Event [+ Condition])
					auto EventTrans = std::make_shared<WEventTransition>(TBinding.Target, CondFunc);

					// 전이 시 실행될 액션들 바인딩
					for (const auto& ActionFactory : TBinding.ActionFactories)
					{
						EventTrans->AddAction(ActionFactory(this));
					}

					NewState->AddEventTransition(EventName, EventTrans);
				}
				else
				{
					// Case 3: 즉시 전이 (Target + Condition)
					// Immediate는 반드시 Condition이 있어야 함
					auto ImmTrans = std::make_shared<WImmediateTransition>(TBinding.Target, CondFunc);

					for (const auto& ActionFactory : TBinding.ActionFactories)
					{
						ImmTrans->AddAction(ActionFactory(this));
					}

					NewState->AddImmediateTransition(ImmTrans);
				}
			}
		}

		for (const auto& Setup : Asset->mComponentSetups)
		{
			if (WActorComponent* Comp = GetWObject<WActorComponent>(Setup.Name))
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
					BindSMEvent(CollisionComp->mOnCollision, "OnHit");
					CollisionComp->mOnCollision.Add(this, &AStateMachineActor::OnHit_Global);
				}
				else if (WProjectileMovementComponent* ProjComp = dynamic_cast<WProjectileMovementComponent*>(Comp))
				{
					BindSMEvent(ProjComp->mOnLockon, "OnLockon");
					BindSMEvent(ProjComp->mOnBounce, "OnBounce");
					BindSMEvent(ProjComp->mOnHomingSuccess, "OnHomingSuccess");
					BindSMEvent(ProjComp->mOnHomingFail, "OnHomingFail");
				}
				else if (WObjectAnimComponent* AnimComp = dynamic_cast<WObjectAnimComponent*>(Comp))
				{
					BindSMEvent(AnimComp->mOnStop, "OnAnimStop");
				}
				else if (WSplineComponent* SplineComp = dynamic_cast<WSplineComponent*>(Comp))
				{
					BindSMEvent(SplineComp->mOnFollowSplineEnd, "OnFollowSplineEnd");
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

void AStateMachineActor::OnHit_Global(WSceneComponent* Instigator, WPhysicsComponent* HittedComponent, XMFLOAT3 ImpulseDir, XMFLOAT3 ImpactPoint, XMFLOAT3 Normal, float Distance, float Damage)
{
	mStateMachine->SendEvent("OnHit");

	if (IHitInterface* HitInterf = dynamic_cast<IHitInterface*>(HittedComponent->GetOwner<AActor>()))
	{
		HitInterf->OnHit(Instigator, HittedComponent, ImpulseDir, ImpactPoint, Normal, Distance, Damage);
	}
}
