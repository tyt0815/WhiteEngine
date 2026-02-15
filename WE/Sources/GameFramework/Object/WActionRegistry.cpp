#include "WActionRegistry.h"
#include "Object.h"
#include "Actor/Actor.h"
#include "World/World.h"
#include "Parser.h"
#include "GameFramework/Interface/CollisionGenerator.h"
#include "Component/SplineComponent.h"
#include "Component/StateMachineComponent.h"

void LogRegisterError(const std::string& ActionName, const std::string& Content)
{
	std::cout << "Action Register Error[" + ActionName + "]" << Content << std::endl;
}

WActionRegistry::WActionRegistry()
{
	//Register_Internal("SetWorldLocation", [](WObject* Target, const WAttributesMap& Attr) {
	//	const std::string TargetName = Attr.at("Target");
	//	// AActor로 캐스팅하여 컴포넌트 탐색 (컴파일 시점에 미리 찾아둠)
	//	AActor* Owner = static_cast<AActor*>(Target);
	//	WSceneComponent* TargetComp = Owner->GetWComponent<WSceneComponent>(TargetName);

	//	// Loc 수식 바인딩
	//	auto LocFunc = WExpressionParser::Bind<XMFLOAT3>(Target, Attr, "Loc", "{0, 0, 0}");

	//	return [TargetComp, LocFunc]() {
	//		if (TargetComp) TargetComp->SetWorldLocation(LocFunc());
	//	};
	//	});
	//Register_Internal("SetRelativeLocation", [](WObject* Target, const WAttributesMap& Attr) {
	//	const std::string TargetName = Attr.at("Target");
	//	AActor* Owner = static_cast<AActor*>(Target);
	//	WSceneComponent* TargetComp = Owner->GetWComponent<WSceneComponent>(TargetName);

	//	auto LocFunc = WExpressionParser::Bind<XMFLOAT3>(Target, Attr, "Loc", "{0, 0, 0}");

	//	return [TargetComp, LocFunc]() {
	//		if (TargetComp) TargetComp->SetRelativeLocation(LocFunc());
	//	};
	//	});
	//Register_Internal("SetWorldRotation", [](WObject* Target, const WAttributesMap& Attr) {
	//	const std::string TargetName = Attr.at("Target");
	//	AActor* Owner = static_cast<AActor*>(Target);
	//	WSceneComponent* TargetComp = Owner->GetWComponent<WSceneComponent>(TargetName);

	//	auto RotFunc = WExpressionParser::Bind<XMFLOAT3>(Target, Attr, "Rot", "{0, 0, 0}");

	//	return [TargetComp, RotFunc]() {
	//		if (TargetComp) TargetComp->SetWorldRotation(RotFunc());
	//	};
	//	});
	//Register_Internal("SetRelativeRotation", [](WObject* Target, const WAttributesMap& Attr) {
	//	const std::string TargetName = Attr.at("Target");
	//	AActor* Owner = static_cast<AActor*>(Target);
	//	WSceneComponent* TargetComp = Owner->GetWComponent<WSceneComponent>(TargetName);

	//	auto RotFunc = WExpressionParser::Bind<XMFLOAT3>(Target, Attr, "Rot", "{0, 0, 0}");

	//	return [TargetComp, RotFunc]() {
	//		if (TargetComp) TargetComp->SetRelativeRotation(RotFunc());
	//	};
	//	});
	//Register_Internal("SetWorldScale", [](WObject* Target, const WAttributesMap& Attr) {
	//	const std::string TargetName = Attr.at("Target");
	//	AActor* Owner = static_cast<AActor*>(Target);
	//	WSceneComponent* TargetComp = Owner->GetWComponent<WSceneComponent>(TargetName);

	//	auto ScaleFunc = WExpressionParser::Bind<XMFLOAT3>(Target, Attr, "Scale", "{1, 1, 1}");

	//	return [TargetComp, ScaleFunc]() {
	//		if (TargetComp) TargetComp->SetWorldScale(ScaleFunc());
	//	};
	//	});
	//Register_Internal("SetRelativeScale", [](WObject* Target, const WAttributesMap& Attr) {
	//	const std::string TargetName = Attr.at("Target");
	//	AActor* Owner = static_cast<AActor*>(Target);
	//	WSceneComponent* TargetComp = Owner->GetWComponent<WSceneComponent>(TargetName);

	//	auto ScaleFunc = WExpressionParser::Bind<XMFLOAT3>(Target, Attr, "Scale", "{1, 1, 1}");

	//	return [TargetComp, ScaleFunc]() {
	//		if (TargetComp) TargetComp->SetRelativeScale(ScaleFunc());
	//	};
	//	});

	//Register_Internal("SpawnActor", [](WObject* Target, const WAttributesMap& Attr) {
	//	std::string ActorName = Attr.at("Name");
	//	AActor* Owner = static_cast<AActor*>(Target);

	//	// 생성 위치/회전/크기 수식 바인딩 (기본값은 루트 컴포넌트 기준)
	//	auto LocFunc = WExpressionParser::Bind<XMFLOAT3>(Target, Attr, "Loc", "Root.GetWorldLocation()");
	//	auto RotFunc = WExpressionParser::Bind<XMFLOAT3>(Target, Attr, "Rot", "Root.GetWorldRotation()");
	//	auto ScaleFunc = WExpressionParser::Bind<XMFLOAT3>(Target, Attr, "Scale", "Root.GetWorldScale()");

	//	return [Owner, ActorName, LocFunc, RotFunc, ScaleFunc]() {
	//		FActorSpawnParameter Param;
	//		Param.Transform.Translation = LocFunc();
	//		Param.Transform.Rotation = RotFunc();
	//		Param.Transform.Scale = ScaleFunc();

	//		// 전역 월드 컨텍스트를 통해 스폰
	//		Owner->GetWorld()->SpawnActorByFactory<AActor>(ActorName, Param);
	//	};
	//	});
	Register_Internal("Destroy", [](WObject* Target, const WAttributesMap& Attr) {
		AActor* Owner = dynamic_cast<AActor*>(Target);

		return [Owner]() {
			if (Owner) Owner->Destroy();
		};
		});

	Register_Internal("Call", [](WObject* Target, const WAttributesMap& Attributes) -> WActionLambda {

		auto it = Attributes.find("Target");
		if (it == Attributes.end())
		{
			LogRegisterError("Call", "Input target name");
			return []() {};
		}

		std::string FullPath = it->second;
		std::string TargetStateName = "";
		std::string TargetEventName = "";

		// [컴파일/로드 시점] 미리 점(.)을 파싱하여 타겟을 분리해둠
		size_t DotPos = FullPath.find('.');
		if (DotPos != std::string::npos)
		{
			TargetStateName = FullPath.substr(0, DotPos);
			TargetEventName = FullPath.substr(DotPos + 1);
		}
		else
		{
			TargetEventName = FullPath;
		}

		// [런타임 시점] 파싱된 결과만 캡처하여 즉시 실행
		return [Target, TargetStateName, TargetEventName]() {
			if (!Target) return;

			AActor* Actor = static_cast<AActor*>(Target);
			if (auto* SM = Actor->GetWComponent<WStateMachineComponent>("StateMachine"))
			{
				if (!TargetStateName.empty())
				{
					// 특정 상태(Base 등)의 이벤트를 직접 지목해서 호출
					if (WState* SpecifiedState = SM->GetState(TargetStateName))
					{
						SpecifiedState->HandleEvent(TargetEventName);
					}
				}
				else
				{
					// 현재 활성화된 상태의 이벤트를 호출
					SM->SendEvent(TargetEventName);
				}
			}
		};
		});

	Register_Internal("Log", [](WObject* Target, const WAttributesMap& Attr) {
		std::string Content = Attr.at("Content");
		return [Content]() {
			std::cout << Content << std::endl;
		};
		});

	//Register_Internal("Register", [](WObject* Target, const WAttributesMap& Attr) {
	//	std::string Name = Attr.at("Name");
	//	// 초기값 평가 (Evaluate는 바인딩 시점 즉시 계산)
	//	WEvalValue InitialValue = WExpressionParser::Evaluate(Target, Attr, "Value", "None");

	//	return [Target, Name, InitialValue]() {
	//		Target->RegisterWProperty(Name, InitialValue);
	//	};
	//	});
	//Register_Internal("Set", [](WObject* Target, const WAttributesMap& Attr) {
	//	std::string Name = Attr.at("Name");
	//	// 수식 바인딩 (실행 시점에 계산될 값)
	//	auto ValueFunc = WExpressionParser::Bind(Target, Attr, "Value", "None");

	//	return [Target, Name, ValueFunc]() {
	//		// WObject 내부의 Variant 기반 프로퍼티 시스템 활용
	//		WEvalValue NewValue = ValueFunc();
	//		Target->SetPropertyValue(Name, NewValue);
	//	};
	//	});

	//Register_Internal("Event", [](WObject* Target, const WAttributesMap& Attr) {
	//	std::string Name = Attr.at("Name");
	//	AActor* Owner = static_cast<AActor*>(Target);

	//	return [Owner, Name]() {
	//		// Owner가 가진 이벤트 맵에서 해당 이벤트를 찾아 실행
	//		Owner->DispatchCustomEvent(Name);
	//	};
	//	});

	//auto RegisterActivateAction = [&](const std::string& Tag, bool bActivate) {
	//	Register_Internal(Tag, [bActivate](WObject* Target, const WAttributesMap& Attr) {
	//		AActor* Owner = static_cast<AActor*>(Target);
	//		WSceneComponent* TargetComp = Owner->GetWComponent<WSceneComponent>(Attr.at("Target"));
	//		auto WithChildFunc = WExpressionParser::Bind<bool>(Target, Attr, "WithChild", "true");

	//		return [TargetComp, WithChildFunc, bActivate]() {
	//			if (!TargetComp) return;
	//			bool bWithChild = WithChildFunc();
	//			if (bActivate)
	//				bWithChild ? TargetComp->ActivateWithChild() : TargetComp->Activate();
	//			else
	//				bWithChild ? TargetComp->DeactivateWithChild() : TargetComp->Deactivate();
	//		};
	//		});
	//};
	//RegisterActivateAction("Activate", true);
	//RegisterActivateAction("Deactivate", false);

	//Register_Internal("FollowSpline", [](WObject* Target, const WAttributesMap& Attr) {
	//	AActor* Owner = static_cast<AActor*>(Target);
	//	WSceneComponent* TargetComp = Owner->GetWComponent<WSceneComponent>(Attr.at("Target"));
	//	WSplineComponent* Spline = Owner->GetWComponent<WSplineComponent>(Attr.at("Spline"));

	//	auto DurationFunc = WExpressionParser::Bind<float>(Target, Attr, "Duration", "1");
	//	auto UseRotFunc = WExpressionParser::Bind<bool>(Target, Attr, "UseRotation", "true");
	//	auto LoopFunc = WExpressionParser::Bind<bool>(Target, Attr, "Loop", "false");

	//	return [Owner, TargetComp, Spline, DurationFunc, UseRotFunc, LoopFunc]() {
	//		if (!TargetComp || !Spline) return;
	//		FSplineFollowInfo Info;
	//		Info.Target = TargetComp;
	//		Info.Spline = Spline;
	//		Info.Duration = DurationFunc();
	//		Info.bUseRotation = UseRotFunc();
	//		Info.bLoop = LoopFunc();
	//		// 액터 인스턴스의 전용 큐에 추가
	//		Owner->AddSplineFollowInfo(std::move(Info));
	//	};
	//	});
	//Register_Internal("Branch", [](WObject* Target, const WAttributesMap& Attr) {
	//	AActor* Owner = static_cast<AActor*>(Target);
	//	auto bCondFunc = WExpressionParser::Bind<bool>(Target, Attr, "Condition", "true");
	//	std::string OnTrue = Attr.at("OnTrue");
	//	std::string OnFalse = Attr.at("OnFalse");

	//	return [Owner, bCondFunc, OnTrue, OnFalse]() {
	//		Owner->DispatchCustomEvent(bCondFunc() ? OnTrue : OnFalse);
	//	};
	//	});
	//Register_Internal("Timer", [](WObject* Target, const WAttributesMap& Attr) {
	//	AActor* Owner = static_cast<AActor*>(Target);
	//	auto TimeFunc = WExpressionParser::Bind<float>(Target, Attr, "Time", "0");
	//	auto bLoopFunc = WExpressionParser::Bind<bool>(Target, Attr, "Loop", "false");
	//	std::string EventName = Attr.at("Event");

	//	return [Owner, TimeFunc, bLoopFunc, EventName]() {
	//		FTimerActionInfo Info;
	//		Info.Event = Owner->GetCustomEvent(EventName);
	//		Info.Time = TimeFunc();
	//		Info.bLoop = bLoopFunc();
	//		Info.ExpireTime = Owner->GetElapsedTime() + Info.Time;
	//		Owner->AddTimerAction(std::move(Info));
	//	};
	//	});
	//Register_Internal("SetUpdateOrder", [](WObject* Target, const WAttributesMap& Attr) {
	//	AActor* Owner = static_cast<AActor*>(Target);
	//	auto TargetNameFunc = WExpressionParser::Bind<std::string>(Target, Attr, "Target", "this_self_none_default");
	//	auto OrderFunc = WExpressionParser::Bind<float>(Target, Attr, "Order", "0.0");

	//	return [Owner, TargetNameFunc, OrderFunc]() {
	//		std::string TName = TargetNameFunc();
	//		WObject* TargetObj = (TName == "this_self_none_default") ? Owner : Owner->GetWComponent(TName);
	//		if (!TargetObj) return;

	//		int TotalOrder = static_cast<int>(OrderFunc());
	//		if (TotalOrder < 0) TotalOrder = 0;

	//		const int PriorityCount = static_cast<int>(ETickPriority::ETP_None);
	//		const int GroupCount = static_cast<int>(ETickGroup::ETG_None);

	//		int GroupIdx = std::min(TotalOrder / PriorityCount, GroupCount - 1);
	//		int PriorityIdx = TotalOrder % PriorityCount;

	//		TargetObj->SetTickGroup(static_cast<ETickGroup>(GroupIdx), static_cast<ETickPriority>(PriorityIdx));
	//	};
	//	});

	//Register_Internal("PlayAnim", [](WObject* Target, const WAttributesMap& Attr) {
	//	AActor* Owner = static_cast<AActor*>(Target);
	//	std::string TargetCompName = Attr.at("Target");

	//	// 런타임 성능을 위해 컴포넌트 포인터를 미리 찾아둠 (컴파일 시점)
	//	auto* Anim = Owner->GetWComponent<WObjectAnimComponent>(TargetCompName);

	//	auto PlayRateFunc = WExpressionParser::Bind<float>(Target, Attr, "PlayRate", "1");
	//	auto LoopFunc = WExpressionParser::Bind<bool>(Target, Attr, "Loop", "false");
	//	auto RootMotionFunc = WExpressionParser::Bind<bool>(Target, Attr, "RootMotion", "false");

	//	// 샘플링 플래그 계산 (컴파일 시점에 1회만 계산)
	//	uint16_t Flags = 0;
	//	auto ParseFlag = [&](const std::string& AttrKey, uint16_t X, uint16_t Y, uint16_t Z) {
	//		auto it = Attr.find(AttrKey);
	//		if (it != Attr.end()) {
	//			if (it->second.find('X') != std::string::npos) Flags |= X;
	//			if (it->second.find('Y') != std::string::npos) Flags |= Y;
	//			if (it->second.find('Z') != std::string::npos) Flags |= Z;
	//		}
	//	};

	//	ParseFlag("Loc", EAnimSampling::LocX, EAnimSampling::LocY, EAnimSampling::LocZ);
	//	ParseFlag("Rot", EAnimSampling::RotX, EAnimSampling::RotY, EAnimSampling::RotZ);
	//	ParseFlag("Scale", EAnimSampling::ScaleX, EAnimSampling::ScaleY, EAnimSampling::ScaleZ);

	//	if (Flags == 0) Flags = EAnimSampling::All;

	//	// 자산 정보가 있는지 확인
	//	bool bHasAsset = Attr.count("Asset") > 0;
	//	std::string AssetName = bHasAsset ? Attr.at("Asset") : "";
	//	std::string AnimName = bHasAsset ? Attr.at("Anim") : "";

	//	return [=]() {
	//		if (!Anim) return;
	//		if (bHasAsset) {
	//			Anim->LoadAndPlay(AssetName, AnimName, PlayRateFunc(), LoopFunc(), Flags, RootMotionFunc());
	//		}
	//		else {
	//			Anim->Play(PlayRateFunc(), LoopFunc(), Flags, RootMotionFunc());
	//		}
	//	};
	//	});

	//// 2. CurveBind: 애니메이션 커브와 프로퍼티 연결
	//Register_Internal("CurveBind", [](WObject* Target, const WAttributesMap& Attr) {
	//	AActor* Owner = static_cast<AActor*>(Target);
	//	auto* Anim = Owner->GetWComponent<WObjectAnimComponent>(Attr.at("Target"));

	//	std::string PropertyName = Attr.at("Property");
	//	// 프로퍼티 포인터 획득 (WObject의 프로퍼티 시스템 활용)
	//	float* PropPtr = std::get<float*>(Owner->GetWPropertyPtr(PropertyName));

	//	auto CurveFunc = WExpressionParser::Bind<std::string>(Target, Attr, "Curve", "");
	//	auto ModifierFunc = WExpressionParser::Bind<bool>(Target, Attr, "Modifier", "false");

	//	return [=]() {
	//		if (!Anim || !PropPtr) return;
	//		// Modifier가 true일 경우 현재 값을 BaseValue로 사용
	//		Anim->BindCurve(CurveFunc(), PropPtr, ModifierFunc(), *PropPtr);
	//	};
	//	});

	//// 3. BindCollision: 이동 컴포넌트와 충돌 컴포넌트 물리적 연결
	//Register_Internal("BindCollision", [](WObject* Target, const WAttributesMap& Attr) {
	//	AActor* Owner = static_cast<AActor*>(Target);
	//	auto* Movement = Owner->GetWComponent<WProjectileMovementComponent>(Attr.at("Movement"));
	//	auto* Collision = Owner->GetWComponent<FCollisionGeneratorBase>(Attr.at("Collision"));

	//	return [=]() {
	//		if (Movement && Collision) {
	//			Movement->BindCollisionEvent(Collision);
	//		}
	//	};
	//	});
}

WActionRegistry::~WActionRegistry()
{

}

void WActionRegistry::Register_Internal(const std::string& Tag, WActionCreator Creator)
{
    mCreators[Tag] = std::move(Creator);
}

WActionLambda WActionRegistry::Create_Internal(const std::string& Tag, WObject* Target, const WAttributesMap& Attributes)
{
    auto it = mCreators.find(Tag);
    if (it != mCreators.end())
    {
        return it->second(Target, Attributes);
    }

    // 에러 핸들링: 등록되지 않은 태그일 경우 빈 함수 반환 혹은 로그
    return nullptr;
}