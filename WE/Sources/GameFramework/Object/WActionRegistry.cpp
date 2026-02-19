#include "WActionRegistry.h"
#include "Object.h"
#include "Actor/Actor.h"
#include "World/World.h"
#include "Parser.h"
#include "GameFramework/Interface/CollisionGenerator.h"
#include "Component/SplineComponent.h"
#include "Component/StateMachineComponent.h"
#include "Component/ObjectAnimComponent.h"
#include "Component/ProjectileMovementComponent.h"

void LogRegisterError(const std::string& ActionName, const std::string& Content)
{
	std::cout << "Action Register Error[" + ActionName + "]" << Content << std::endl;
}

WActionRegistry::WActionRegistry()
{
	Register_Internal("SetWorldLocation", [](WObject* Target, const WAttributesMap& Attr, const std::vector<WActionFactory>& SubActionFactories) {
		const std::string TargetName = Attr.at("Target");
		// AActor로 캐스팅하여 컴포넌트 탐색 (컴파일 시점에 미리 찾아둠)
		AActor* Owner = static_cast<AActor*>(Target);
		WSceneComponent* TargetComp = Owner->GetWComponent<WSceneComponent>(TargetName);

		// Loc 수식 바인딩
		auto LocFunc = WExpressionParser::Bind<XMFLOAT3>(Target, Attr, "Loc", "{0, 0, 0}");

		return [TargetComp, LocFunc]() {
			if (TargetComp) TargetComp->SetWorldLocation(LocFunc());
		};
		});
	Register_Internal("SetRelativeLocation", [](WObject* Target, const WAttributesMap& Attr, const std::vector<WActionFactory>& SubActionFactories) {
		const std::string TargetName = Attr.at("Target");
		AActor* Owner = static_cast<AActor*>(Target);
		WSceneComponent* TargetComp = Owner->GetWComponent<WSceneComponent>(TargetName);

		auto LocFunc = WExpressionParser::Bind<XMFLOAT3>(Target, Attr, "Loc", "{0, 0, 0}");

		return [TargetComp, LocFunc]() {
			if (TargetComp) TargetComp->SetRelativeLocation(LocFunc());
		};
		});
	Register_Internal("SetWorldRotation", [](WObject* Target, const WAttributesMap& Attr, const std::vector<WActionFactory>& SubActionFactories) {
		const std::string TargetName = Attr.at("Target");
		AActor* Owner = static_cast<AActor*>(Target);
		WSceneComponent* TargetComp = Owner->GetWComponent<WSceneComponent>(TargetName);

		auto RotFunc = WExpressionParser::Bind<XMFLOAT3>(Target, Attr, "Rot", "{0, 0, 0}");

		return [TargetComp, RotFunc]() {
			if (TargetComp) TargetComp->SetWorldRotation(RotFunc());
		};
		});
	Register_Internal("SetRelativeRotation", [](WObject* Target, const WAttributesMap& Attr, const std::vector<WActionFactory>& SubActionFactories) {
		const std::string TargetName = Attr.at("Target");
		AActor* Owner = static_cast<AActor*>(Target);
		WSceneComponent* TargetComp = Owner->GetWComponent<WSceneComponent>(TargetName);

		auto RotFunc = WExpressionParser::Bind<XMFLOAT3>(Target, Attr, "Rot", "{0, 0, 0}");

		return [TargetComp, RotFunc]() {
			if (TargetComp) TargetComp->SetRelativeRotation(RotFunc());
		};
		});
	Register_Internal("SetWorldScale", [](WObject* Target, const WAttributesMap& Attr, const std::vector<WActionFactory>& SubActionFactories) {
		const std::string TargetName = Attr.at("Target");
		AActor* Owner = static_cast<AActor*>(Target);
		WSceneComponent* TargetComp = Owner->GetWComponent<WSceneComponent>(TargetName);

		auto ScaleFunc = WExpressionParser::Bind<XMFLOAT3>(Target, Attr, "Scale", "{1, 1, 1}");

		return [TargetComp, ScaleFunc]() {
			if (TargetComp) TargetComp->SetWorldScale(ScaleFunc());
		};
		});
	Register_Internal("SetRelativeScale", [](WObject* Target, const WAttributesMap& Attr, const std::vector<WActionFactory>& SubActionFactories) {
		const std::string TargetName = Attr.at("Target");
		AActor* Owner = static_cast<AActor*>(Target);
		WSceneComponent* TargetComp = Owner->GetWComponent<WSceneComponent>(TargetName);

		auto ScaleFunc = WExpressionParser::Bind<XMFLOAT3>(Target, Attr, "Scale", "{1, 1, 1}");

		return [TargetComp, ScaleFunc]() {
			if (TargetComp) TargetComp->SetRelativeScale(ScaleFunc());
		};
		});

	Register_Internal("SpawnActor", [](WObject* Target, const WAttributesMap& Attr, const std::vector<WActionFactory>& SubActionFactories) {
		std::string ActorName = Attr.at("Name");
		AActor* Owner = static_cast<AActor*>(Target);

		// 생성 위치/회전/크기 수식 바인딩 (기본값은 루트 컴포넌트 기준)
		auto LocFunc = WExpressionParser::Bind<XMFLOAT3>(Target, Attr, "Loc", "Root.GetWorldLocation()");
		auto RotFunc = WExpressionParser::Bind<XMFLOAT3>(Target, Attr, "Rot", "Root.GetWorldRotation()");
		auto ScaleFunc = WExpressionParser::Bind<XMFLOAT3>(Target, Attr, "Scale", "Root.GetWorldScale()");

		return [Owner, ActorName, LocFunc, RotFunc, ScaleFunc]() {
			FActorSpawnParameter Param;
			Param.Transform.Translation = LocFunc();
			Param.Transform.Rotation = RotFunc();
			Param.Transform.Scale = ScaleFunc();

			// 전역 월드 컨텍스트를 통해 스폰
			Owner->GetWorld()->SpawnActorByFactory<AActor>(ActorName, Param);
		};
		});
	Register_Internal("Destroy", [](WObject* Target, const WAttributesMap& Attr, const std::vector<WActionFactory>& SubActionFactories) {
		AActor* Owner = dynamic_cast<AActor*>(Target);

		return [Owner]() {
			if (Owner) Owner->Destroy();
		};
		});

	Register_Internal("Call", [](WObject* Target, const WAttributesMap& Attributes, const std::vector<WActionFactory>& SubActionFactories) -> WActionLambda {
		AActor* Actor = static_cast<AActor*>(Target);
		if (Actor == nullptr)
		{
			return []() {};
		}
		auto* SM = Actor->GetWComponent<WStateMachineComponent>("StateMachine");
		if(SM == nullptr)
		{
			return []() {};
		}

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
		return [SM, TargetStateName, TargetEventName]() {
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
		};
		});

	Register_Internal("Log", [](WObject* Target, const WAttributesMap& Attr, const std::vector<WActionFactory>& SubActionFactories) {
		std::string Content = Attr.at("Content");
		return [Content]() {
			std::cout << Content << std::endl;
		};
		});

	Register_Internal("Register", [](WObject* Target, const WAttributesMap& Attr, const std::vector<WActionFactory>& SubActionFactories) {
		std::string Name = Attr.at("Name");
		// 초기값 평가 (Evaluate는 바인딩 시점 즉시 계산)
		WEvalValue InitialValue = WExpressionParser::Evaluate(Target, Attr, "Value", "None");

		return [Target, Name, InitialValue]() {
			Target->RegisterWProperty(Name, InitialValue);
		};
		});
	Register_Internal("Set", [](WObject* Target, const WAttributesMap& Attr, const std::vector<WActionFactory>& SubActionFactories) {
		std::string Name = Attr.at("Name");
		// 수식 바인딩 (실행 시점에 계산될 값)
		auto ValueFunc = WExpressionParser::Bind(Target, Attr, "Value", "None");

		return [Target, Name, ValueFunc]() {
			// WObject 내부의 Variant 기반 프로퍼티 시스템 활용
			WEvalValue NewValue = ValueFunc();
			Target->SetWPropertyValue(Name, NewValue);
		};
		});

	auto RegisterActivateAction = [&](const std::string& Tag, bool bActivate) {
		Register_Internal(Tag, [bActivate](WObject* Target, const WAttributesMap& Attr, const std::vector<WActionFactory>& SubActionFactories) {
			AActor* Owner = static_cast<AActor*>(Target);
			WSceneComponent* TargetComp = Owner->GetWComponent<WSceneComponent>(Attr.at("Target"));
			auto WithChildFunc = WExpressionParser::Bind<bool>(Target, Attr, "WithChild", "true");

			return [TargetComp, WithChildFunc, bActivate]() {
				if (!TargetComp) return;
				bool bWithChild = WithChildFunc();
				if (bActivate)
					bWithChild ? TargetComp->ActivateWithChild() : TargetComp->Activate();
				else
					bWithChild ? TargetComp->DeactivateWithChild() : TargetComp->Deactivate();
			};
			});
	};
	RegisterActivateAction("Activate", true);
	RegisterActivateAction("Deactivate", false);

	Register_Internal("FollowSpline", [](WObject* Target, const WAttributesMap& Attr, const std::vector<WActionFactory>& SubActionFactories) -> WActionLambda {
		AActor* Owner = static_cast<AActor*>(Target);

		// 1. 따라갈 대상 (기본값은 액터의 RootComponent)
		std::string TargetName = Attr.count("Target") ? Attr.at("Target") : "";
		WSceneComponent* TargetComp = TargetName.empty() ?
			Owner->GetRootComponent() : Owner->GetWComponent<WSceneComponent>(TargetName);

		// 2. 어떤 스플라인을 따라갈 것인가?
		auto itSpline = Attr.find("Spline");
		if (itSpline == Attr.end()) return []() {}; // 스플라인 지정 없으면 무시
		WSplineComponent* Spline = Owner->GetWComponent<WSplineComponent>(itSpline->second);

		// 3. 수식 바인딩
		auto DurationFunc = WExpressionParser::Bind<float>(Target, Attr, "Duration", "1");
		auto UseRotFunc = WExpressionParser::Bind<bool>(Target, Attr, "UseRotation", "true");
		auto LoopFunc = WExpressionParser::Bind<bool>(Target, Attr, "Loop", "false");

		return [TargetComp, Spline, DurationFunc, UseRotFunc, LoopFunc]() {
			if (!TargetComp || !Spline) return;

			WSplineComponent::FSplineFollower Follower;
			Follower.Target = TargetComp;
			Follower.Duration = DurationFunc();
			Follower.bUseRotation = UseRotFunc();
			Follower.bLoop = LoopFunc();
			Follower.CurrentDistance = 0.0f;

			// 스플라인 컴포넌트에게 팔로워 등록 요청
			Spline->AddFollower(Follower);
		};
		});

	Register_Internal("Branch", [](WObject* Target, const WAttributesMap& Attr, const std::vector<WActionFactory>& SubActionFactories) -> WActionLambda {
		AActor* Actor = static_cast<AActor*>(Target);
		if (Actor == nullptr)
		{
			return []() {};
		}
		auto* SM = Actor->GetWComponent<WStateMachineComponent>("StateMachine");
		if (SM == nullptr)
		{
			return []() {};
		}
		
		auto bCondFunc = WExpressionParser::Bind<bool>(Target, Attr, "Condition", "true");
		std::string OnTrue = Attr.at("OnTrue");
		std::string OnFalse = Attr.at("OnFalse");

		return [SM, bCondFunc, OnTrue, OnFalse]() {
			if (bCondFunc())
			{
				SM->SendEvent(OnTrue);
			}
			else
			{
				SM->SendEvent(OnFalse);
			}
		};
		});

	Register_Internal("Timer", [](WObject* Target, const WAttributesMap& Attr, const std::vector<WActionFactory>& SubActionFactories) -> WActionLambda {
		WWorld* World = GetWorld();
		AActor* Owner = static_cast<AActor*>(Target);

		// 1. 속성 파싱 (시간, 루프 여부, 이벤트 이름)
		auto TimeFunc = WExpressionParser::Bind<float>(Target, Attr, "Time", "0");
		auto bLoopFunc = WExpressionParser::Bind<bool>(Target, Attr, "Loop", "false");

		auto it = Attr.find("Call");
		if (it == Attr.end()) return []() {}; // 이벤트 이름 없으면 무시
		std::string EventName = it->second;

		// 2. 실행 시점 람다 반환
		return [World, Owner, TimeFunc, bLoopFunc, EventName]() {
			if (!World || !Owner) return;

			float Time = TimeFunc();
			bool bLoop = bLoopFunc();

			// 3. 월드 타이머 시스템에 등록
			// 델리게이트 내부에서 Owner의 이벤트를 전송하도록 구성
			World->AddWorldTimer(Time, bLoop, [Owner, EventName]() {
				if (Owner)
				{
					// 액터의 스테이트 머신이나 커스텀 이벤트 핸들러에 신호 발송
					// 앞서 만든 SendEvent나 Dispatch를 활용합니다.
					if (auto* SM = Owner->GetWComponent<WStateMachineComponent>("StateMachine"))
					{
						SM->SendEvent(EventName);
						return true;
					}
				}

				return false;
				});
		};
		});

	Register_Internal("SetUpdateOrder", [](WObject* Target, const WAttributesMap& Attr, const std::vector<WActionFactory>& SubActionFactories) {
		AActor* Owner = static_cast<AActor*>(Target);
		auto TargetNameFunc = WExpressionParser::Bind<std::string>(Target, Attr, "Target", "this_self_none_default");
		auto OrderFunc = WExpressionParser::Bind<float>(Target, Attr, "Order", "0.0");

		return [Owner, TargetNameFunc, OrderFunc]() {
			std::string TName = TargetNameFunc();
			WObject* TargetObj = (TName == "this_self_none_default") ? Owner : Owner->GetWComponent<WObject>(TName);
			if (!TargetObj) return;

			int TotalOrder = static_cast<int>(OrderFunc());
			if (TotalOrder < 0) TotalOrder = 0;

			const int PriorityCount = static_cast<int>(ETickPriority::ETP_None);
			const int GroupCount = static_cast<int>(ETickGroup::ETG_None);

			int GroupIdx = min(TotalOrder / PriorityCount, GroupCount - 1);
			int PriorityIdx = TotalOrder % PriorityCount;

			TargetObj->SetTickGroup(static_cast<ETickGroup>(GroupIdx), static_cast<ETickPriority>(PriorityIdx));
		};
		});

	Register_Internal("PlayAnim", [](WObject* Target, const WAttributesMap& Attr, const std::vector<WActionFactory>& SubActionFactories) {
		AActor* Owner = static_cast<AActor*>(Target);
		std::string TargetCompName = Attr.at("Target");

		// 런타임 성능을 위해 컴포넌트 포인터를 미리 찾아둠 (컴파일 시점)
		auto* Anim = Owner->GetWComponent<WObjectAnimComponent>(TargetCompName);

		auto PlayRateFunc = WExpressionParser::Bind<float>(Target, Attr, "PlayRate", "1");
		auto LoopFunc = WExpressionParser::Bind<bool>(Target, Attr, "Loop", "false");
		auto RootMotionFunc = WExpressionParser::Bind<bool>(Target, Attr, "RootMotion", "false");

		// 샘플링 플래그 계산 (컴파일 시점에 1회만 계산)
		uint16_t Flags = 0;
		auto ParseFlag = [&](const std::string& AttrKey, uint16_t X, uint16_t Y, uint16_t Z) {
			auto it = Attr.find(AttrKey);
			if (it != Attr.end()) {
				if (it->second.find('X') != std::string::npos) Flags |= X;
				if (it->second.find('Y') != std::string::npos) Flags |= Y;
				if (it->second.find('Z') != std::string::npos) Flags |= Z;
			}
		};

		ParseFlag("Loc", EAnimSampling::LocX, EAnimSampling::LocY, EAnimSampling::LocZ);
		ParseFlag("Rot", EAnimSampling::RotX, EAnimSampling::RotY, EAnimSampling::RotZ);
		ParseFlag("Scale", EAnimSampling::ScaleX, EAnimSampling::ScaleY, EAnimSampling::ScaleZ);

		if (Flags == 0) Flags = EAnimSampling::All;

		// 자산 정보가 있는지 확인
		bool bHasAsset = Attr.count("Asset") > 0;
		std::string AssetName = bHasAsset ? Attr.at("Asset") : "";
		std::string AnimName = bHasAsset ? Attr.at("Anim") : "";

		return [=]() {
			if (!Anim) return;
			if (bHasAsset) {
				Anim->LoadAndPlay(AssetName, AnimName, PlayRateFunc(), LoopFunc(), Flags, RootMotionFunc());
			}
			else {
				Anim->Play(PlayRateFunc(), LoopFunc(), Flags, RootMotionFunc());
			}
		};
		});

	// 2. CurveBind: 애니메이션 커브와 프로퍼티 연결
	Register_Internal("CurveBind", [](WObject* Target, const WAttributesMap& Attr, const std::vector<WActionFactory>& SubActionFactories) {
		AActor* Owner = static_cast<AActor*>(Target);
		auto* Anim = Owner->GetWComponent<WObjectAnimComponent>(Attr.at("Target"));

		std::string PropertyName = Attr.at("Property");
		// 프로퍼티 포인터 획득 (WObject의 프로퍼티 시스템 활용)
		float* PropPtr = std::get<float*>(Owner->GetWPropertyPtr(PropertyName));

		auto CurveFunc = WExpressionParser::Bind<std::string>(Target, Attr, "Curve", "");
		auto ModifierFunc = WExpressionParser::Bind<bool>(Target, Attr, "Modifier", "false");

		return [=]() {
			if (!Anim || !PropPtr) return;
			// Modifier가 true일 경우 현재 값을 BaseValue로 사용
			Anim->BindCurve(CurveFunc(), PropPtr, ModifierFunc(), *PropPtr);
		};
		});

	// 3. BindCollision: 이동 컴포넌트와 충돌 컴포넌트 물리적 연결
	Register_Internal("BindCollision", [](WObject* Target, const WAttributesMap& Attr, const std::vector<WActionFactory>& SubActionFactories) {
		AActor* Owner = static_cast<AActor*>(Target);
		auto* Movement = Owner->GetWComponent<WProjectileMovementComponent>(Attr.at("Movement"));
		auto* Collision = Owner->GetWComponent<FCollisionGeneratorBase>(Attr.at("Collision"));

		return [=]() {
			if (Movement && Collision) {
				Movement->BindCollisionEvent(Collision);
			}
		};
		});

	Register_Internal("ChangeState", [](WObject* Target, const WAttributesMap& Attr, const std::vector<WActionFactory>& SubActionFactories) -> WActionLambda {
		// 1. 타겟 액터와 스테이트 머신 컴포넌트 확보
		AActor* Actor = static_cast<AActor*>(Target);
		auto* SM = Actor->GetWComponent<WStateMachineComponent>("StateMachine");
		if (SM == nullptr)
		{
			return []() {};
		}

		// 상태 이름 추출 (기본값은 빈 문자열)
		auto it = Attr.find("Name");
		if (it == Attr.end()) return []() {};

		std::string NextStateName = it->second;

		// 2. 실행 시점 람다
		return [SM, NextStateName]() {
			SM->TransitionTo(NextStateName);
		};
		});

	Register_Internal("AddForce", [](WObject* Target, const WAttributesMap& Attr, const std::vector<WActionFactory>& SubActionFactories) -> WActionLambda {
		AActor* Owner = static_cast<AActor*>(Target);

		std::string CompName = Attr.count("Target") ? Attr.at("Target") : "";

		// 2. 힘(Force) 값을 표현식으로 바인딩 (XMFLOAT3 타입)
		auto ForceFunc = WExpressionParser::Bind<XMFLOAT3>(Target, Attr, "Force", "{0, 0, 0}");

		return [Owner, CompName, ForceFunc]() {
			if (!Owner) return;

			WProjectileMovementComponent* ProjectileComp = nullptr;

			if (!CompName.empty()) 
			{
				ProjectileComp = Owner->GetWComponent<WProjectileMovementComponent>(CompName);

				if (ProjectileComp) 
				{
					ProjectileComp->AddForce(ForceFunc());
				}
			}

			
		};
		});


	Register_Internal("SpawnProjectile", [](WObject* Target, const WAttributesMap& Attr, const std::vector<WActionFactory>& SubActionFactories) -> WActionLambda {
		std::string ActorName = Attr.at("Name");
		AActor* Owner = static_cast<AActor*>(Target);
		WWorld* World = GetWorld();

		// 1. 공통 속성 추출
		int Count = Attr.count("Count") ? std::stoi(Attr.at("Count")) : 1;
		float Interval = Attr.count("Interval") ? std::stof(Attr.at("Interval")) : 0.0f;
		std::string Strategy = Attr.count("Strategy") ? Attr.at("Strategy") : "Direct";

		// 2. 수식 바인딩 (Loc, Rot은 모든 전략 공통)
		auto LocFunc = WExpressionParser::Bind<XMFLOAT3>(Target, Attr, "Loc", "Root.GetWorldLocation()");
		auto RotFunc = WExpressionParser::Bind<XMFLOAT3>(Target, Attr, "Rot", "Root.GetWorldRotation()");
		auto RangeFunc = WExpressionParser::Bind<float>(Target, Attr, "Range", "0");

		// 3. 전략별 "오프셋/회전 계산 함수" 생성 (컴파일 시점에 람다 확정)
		// 인자: index, baseLoc, baseRot, range
		using TCalculator = std::function<std::pair<XMFLOAT3, XMFLOAT3>(int, const XMFLOAT3&, const XMFLOAT3&, float)>;
		TCalculator CalcFunc;

		if (Strategy == "Fan") {
			CalcFunc = [Count](int i, const XMFLOAT3& L, const XMFLOAT3& R, float Range) {
				float StartAngle = -Range * 0.5f;
				float CurrentAngle = (i == 0 && Range == 0) ? 0 : StartAngle + (Range / (Count - 1) * i); // 실제로는 루프 밖에서 계산된 Step 적용 권장
				// 단순화를 위해 람다 내부 캡처로 Step 미리 계산 가능
				return std::make_pair(L, XMFLOAT3(R.x, R.y + CurrentAngle, R.z));
			};
		}
		else if (Strategy == "Circle") {
			CalcFunc = [Count](int i, const XMFLOAT3& L, const XMFLOAT3& R, float Range) {
				float CurrentAngle = (360.0f / Count) * i; // 실제 갯수(Count) 캡처 필요
				return std::make_pair(L, XMFLOAT3(R.x, R.y + CurrentAngle, R.z));
			};
		}
		else if (Strategy == "Line") {
			// 1. 속성 추가 파싱 (Ratio: 0.5면 중앙 정렬, Angle: 배치 선의 회전)
			float Ratio = Attr.count("Ratio") ? std::stof(Attr.at("Ratio")) : 0.5f;
			float LineAngle = Attr.count("LineAngle") ? std::stof(Attr.at("LineAngle")) : 0.0f;
			int TotalCount = Count; // 전체 개수 캡처

			CalcFunc = [Ratio, LineAngle, TotalCount](int i, const XMFLOAT3& L, const XMFLOAT3& R, float Range) {
				// R(회전값)로부터 현재 바라보는 방향의 Rotation Matrix 생성
				XMMATRIX RotMatrix = XMMatrixRotationRollPitchYaw(
					XMConvertToRadians(R.x),
					XMConvertToRadians(R.y),
					XMConvertToRadians(R.z)
				);

				// 기본 축 추출 (Right, Up)
				XMVECTOR RightDir = RotMatrix.r[0]; // 로컬 Right
				XMVECTOR UpDir = RotMatrix.r[1];    // 로컬 Up

				// LineAngle(배치 회전) 적용: 0이면 Right방향(ㅡ), 90이면 Up방향(ㅣ)
				XMVECTOR vRad = XMVectorReplicate(XMConvertToRadians(LineAngle));
				XMVECTOR LayoutDir = XMVectorCos(vRad) * RightDir + XMVectorSin(vRad) * UpDir;

				// Ratio 적용: i=0일 때의 시작 오프셋 계산
				// Ratio 0.5일 때: i=0은 -0.5 * Range * (Total-1) 위치에서 시작
				float StartOffset = -Ratio * (TotalCount - 1) * Range;
				float CurrentOffset = StartOffset + (i * Range);

				XMVECTOR vLoc = XMLoadFloat3(&L);
				XMFLOAT3 FinalLoc;
				XMStoreFloat3(&FinalLoc, vLoc + (LayoutDir * CurrentOffset));

				return std::make_pair(FinalLoc, R);
			};
		}
		else { // Direct
			CalcFunc = [](int i, const XMFLOAT3& L, const XMFLOAT3& R, float Range) { return std::make_pair(L, R); };
		}

		// 최종 실행 람다
		return [Owner, World, ActorName, Count, Interval, LocFunc, RotFunc, RangeFunc, CalcFunc, SubActionFactories]() {
			XMFLOAT3 BaseLoc = LocFunc();
			XMFLOAT3 BaseRot = RotFunc();
			float Range = RangeFunc();

			auto SpawnLogic = [World, ActorName, CalcFunc, BaseLoc, BaseRot, Range, &SubActionFactories](int idx) {
				auto [FinalLoc, FinalRot] = CalcFunc(idx, BaseLoc, BaseRot, Range);
				FActorSpawnParameter Param;
				Param.Transform.Translation = FinalLoc;
				Param.Transform.Rotation = FinalRot;
				AActor* Projectile = World->SpawnActorByFactory<AActor>(ActorName, Param).lock().get();
				for (auto& ActionFactory : SubActionFactories)
				{
					ActionFactory(Projectile)();
				}
			};

			if (Interval <= 0.0f) {
				// 즉시 발사
				for (int i = 0; i < Count; ++i) SpawnLogic(i);
			}
			else {
				// 시간차 발사 (Interval 옵션)
				for (int i = 0; i < Count; ++i) {
					World->AddWorldTimer(Interval * i, false, [idx = i, SpawnLogic]() 
						{
						SpawnLogic(idx);
						return false; // 단발성
						});
				}
			}
		};
		});
}

WActionRegistry::~WActionRegistry()
{

}

void WActionRegistry::Register_Internal(const std::string& Tag, WActionCreator Creator)
{
    mCreators[Tag] = std::move(Creator);
}

WActionLambda WActionRegistry::Create_Internal(const std::string& Tag, WObject* Target, const WAttributesMap& Attributes, const std::vector<WActionFactory>& SubActionFactories)
{
    auto it = mCreators.find(Tag);
    if (it != mCreators.end())
    {
        return it->second(Target, Attributes, SubActionFactories);
    }

    // 에러 핸들링: 등록되지 않은 태그일 경우 빈 함수 반환 혹은 로그
    return nullptr;
}