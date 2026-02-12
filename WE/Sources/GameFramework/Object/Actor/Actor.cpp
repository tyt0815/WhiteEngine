#include "Actor.h"
#include "Component/SceneComponent.h"
#include "Component/StaticMeshComponent.h"
#include "Component/SplineComponent.h"
#include "Component/ObjectAnimComponent.h"
#include "World/World.h"
#include "Asset/BlueprintAsset.h"
#include "Parser.h"

unsigned int g_ActorCounter = 0;

AActor::AActor():
	mActorCounter(++g_ActorCounter)
{
	TWeakPtr<WSceneComponent> DummyRoot = CreateComponent<WSceneComponent>()->GetWeakPtr<WSceneComponent>();

	SetRootComponent(DummyRoot);

	////////////////////////////////////////////////////////////////////////////////////////////////
	// 
	// WProperty
	// 
	////////////////////////////////////////////////////////////////////////////////////////////////

	////////////////////////////////////////////////////////////////////////////////////////////////
	// WProperty End
	////////////////////////////////////////////////////////////////////////////////////////////////


	////////////////////////////////////////////////////////////////////////////////////////////////
	// 
	// WComponent
	// 
	////////////////////////////////////////////////////////////////////////////////////////////////
	

	RegisterWComponentFactory("Mesh", [this](const WAttributesMap& Attributes)
		{
			WStaticMeshComponent* Comp = this->CreateComponent<WStaticMeshComponent>();

			// Static Mesh 에셋
			ApplyAttribute<std::string>(Attributes, "Asset", [&](const std::string& v) {
				Comp->SetStaticMesh(v);
				});

			return Comp;
		});

	RegisterWComponentFactory("Spline", [this](const WAttributesMap& Attributes)
		{
			WSplineComponent* Comp = this->CreateComponent<WSplineComponent>();

			ApplyAttribute<std::string>(Attributes, "Asset", [&](const std::string& v) {
				Comp->LoadSplineFromAsset(v);
				});

			return Comp;
		});

	RegisterWComponentFactory("Dummy", [this](auto&& Attributes)
		{
			return this->CreateComponent<WSceneComponent>();
		});

	////////////////////////////////////////////////////////////////////////////////////////////////
	// WComponent End
	////////////////////////////////////////////////////////////////////////////////////////////////

	////////////////////////////////////////////////////////////////////////////////////////////////
	// 
	// WAction
	// 
	////////////////////////////////////////////////////////////////////////////////////////////////

	RegisterWActionFactory("Event", [this](const WAttributesMap& Attributes) {
		std::string Name = Attributes.at("Name");
		return [this, Name]() { this->mCustomEventsMap[Name]->Dispatch(); };
		});	

	RegisterWActionFactory("Register", [this](auto&& Attributes)
		{
			std::string Name = Attributes.at("Name");
			WEvalValue Value = WExpressionParser::Evaluate(this, Attributes, "Value", "None");
			mCustomWProperies.push_back(MakeUnique<WEvalValue>(Value));
			WEvalValue* ValuePtr = mCustomWProperies.back().get();
			return [this, Name, ValuePtr]() 
			{ 
				RegisterWProperty(Name, *ValuePtr); 
			};
		});

	RegisterWActionFactory("Set", [this](const WAttributesMap& Attributes) {

		std::string Name = Attributes.at("Name");
		auto ValueFunc = WExpressionParser::Bind(this, Attributes, "Value", "None");

		return [this, Name, ValueFunc]() {
			WSourceRef Target = mWPropertiesMap[Name];
			WEvalValue Value = ValueFunc();
			std::visit([](auto&& TargetPtr, auto&& SourceValue) {
				using TargetType = std::remove_pointer_t<std::decay_t<decltype(TargetPtr)>>;
				using SourceType = std::decay_t<decltype(SourceValue)>;

				if constexpr (std::is_same_v<TargetType, SourceType>) {
					if (TargetPtr) *TargetPtr = SourceValue;
				}
				}, Target, Value);
		};
		});

	RegisterWActionFactory("Destroy", [this](const WAttributesMap& Attributes) {
		return [this]() { this->Destroy(); };
		});

	RegisterWActionFactory("SpawnActor", [this](const WAttributesMap& Attributes)
		{
			std::string Name = Attributes.at("Name");
			
			auto LocFunc = WExpressionParser::Bind<XMFLOAT3>(this, Attributes, "Loc", "Root.GetWorldLocation()");
			auto RotFunc = WExpressionParser::Bind<XMFLOAT3>(this, Attributes, "Rot", "Root.GetWorldRotation()");
			auto ScaleFunc = WExpressionParser::Bind<XMFLOAT3>(this, Attributes, "Scale", "Root.GetWorldScale()");

			return [=]()
			{
				FActorSpawnParameter Param;
				Param.Transform.Translation = LocFunc();
				Param.Transform.Rotation = RotFunc();
				Param.Transform.Scale = ScaleFunc();
				GetWorld()->SpawnActorByFactory<AActor>(Name, Param);
			};
		});

	RegisterWActionFactory("Activate", [this](auto&& Attributes)
		{
			if (Attributes.count("Target") == 0)
			{
				ShowMessageBox(L"Action::Activate: 타겟을 설정해 주세요");
				assert(false);
			}
			const std::string& Target = Attributes.at("Target");
			WSceneComponent* TargetComp = GetWComponent(Target);

			auto WithChildFunc = WExpressionParser::Bind<bool>(this, Attributes, "WithChild", "true");
			
			return [=]() {
				if (WithChildFunc())
				{
					TargetComp->ActivateWithChild();
				}
				else
				{
					TargetComp->Activate();
				}
			};
		});

	RegisterWActionFactory("Deactivate", [this](auto&& Attributes)
		{
			if (Attributes.count("Target") == 0)
			{
				ShowMessageBox(L"Action::Deactivate: 타겟을 설정해 주세요");
				assert(false);
			}

			const std::string& Target = Attributes.at("Target");
			WSceneComponent* TargetComp = GetWComponent(Target);

			auto WithChildFunc = WExpressionParser::Bind<bool>(this, Attributes, "WithChild", "true");

			return [=]() {
				if (WithChildFunc())
				{
					TargetComp->DeactivateWithChild();
				}
				else
				{
					TargetComp->Deactivate();
				}
			};
		});

	RegisterWActionFactory("FollowSpline", [this](auto&& Attributes)
		{
			const std::string TargetName = Attributes.at("Target");
			const std::string SplineName = Attributes.at("Spline");
			WSceneComponent* Target = GetWComponent(TargetName);
			WSplineComponent* Spline = GetWComponent<WSplineComponent>(SplineName);

			auto DurationFunc = WExpressionParser::Bind<float>(this, Attributes,"Duration", "1");
			auto UseRotationFunc = WExpressionParser::Bind<bool>(this, Attributes, "UseRotation", "true");
			auto LoopFunc = WExpressionParser::Bind<bool>(this, Attributes, "Loop", "false");

			return [=]() 
			{
				FSplineFollowInfo Info;
				Info.Target = Target;
				Info.Spline = Spline;
				Info.Duration = DurationFunc();
				Info.bUseRotation = UseRotationFunc();
				Info.bLoop = LoopFunc();
				mSplineFollowInfos.push_back(Info);
			};
		});

	RegisterWActionFactory("Branch", [this](auto&& Attributes)
		{
			auto bConditionFunc = WExpressionParser::Bind<bool>(this, Attributes, "Condition", "true");
			const std::string OnTrueEventName = Attributes.at("OnTrue");
			const std::string OnFalseEventName = Attributes.at("OnFalse");
			return [=]()
			{
				if (bConditionFunc())
				{
					this->mCustomEventsMap[OnTrueEventName]->Dispatch();
				}
				else
				{
					this->mCustomEventsMap[OnFalseEventName]->Dispatch();
				}
			};
		});

	RegisterWActionFactory("SetWorldLocation", [this](auto&& Attributes)
		{
			const std::string TargetName = Attributes.at("Target");
			WSceneComponent* TargetComp = GetWComponent<WSceneComponent>(TargetName);
			auto LocFunc = WExpressionParser::Bind<XMFLOAT3>(this, Attributes, "Loc", "{0, 0, 0}");
			return [TargetComp, LocFunc]()
			{
				TargetComp->SetWorldLocation(LocFunc());
			};
		});
	RegisterWActionFactory("SetRelativeLocation", [this](auto&& Attributes)
		{
			const std::string TargetName = Attributes.at("Target");
			WSceneComponent* TargetComp = GetWComponent<WSceneComponent>(TargetName);
			auto LocFunc = WExpressionParser::Bind<XMFLOAT3>(this, Attributes, "Loc", "{0, 0, 0}");
			return [TargetComp, LocFunc]()
			{
				TargetComp->SetRelativeLocation(LocFunc());
			};
		});
	RegisterWActionFactory("SetWorldRotation", [this](auto&& Attributes)
		{
			const std::string TargetName = Attributes.at("Target");
			WSceneComponent* TargetComp = GetWComponent<WSceneComponent>(TargetName);
			auto RotFunc = WExpressionParser::Bind<XMFLOAT3>(this, Attributes, "Rot", "{0, 0, 0}");
			return [TargetComp, RotFunc]()
			{
				TargetComp->SetWorldRotation(RotFunc());
			};
		});
	RegisterWActionFactory("SetRelativeRotation", [this](auto&& Attributes)
		{
			const std::string TargetName = Attributes.at("Target");
			WSceneComponent* TargetComp = GetWComponent<WSceneComponent>(TargetName);
			auto RotFunc = WExpressionParser::Bind<XMFLOAT3>(this, Attributes, "Rot", "{0, 0, 0}");
			return [TargetComp, RotFunc]()
			{
				TargetComp->SetRelativeRotation(RotFunc());
			};
		});
	RegisterWActionFactory("SetWorldScale", [this](auto&& Attributes)
		{
			const std::string TargetName = Attributes.at("Target");
			WSceneComponent* TargetComp = GetWComponent<WSceneComponent>(TargetName);
			auto ScaleFunc = WExpressionParser::Bind<XMFLOAT3>(this, Attributes, "Scale", "{1, 1, 1}");
			return [TargetComp, ScaleFunc]()
			{
				TargetComp->SetWorldScale(ScaleFunc());
			};
		});
	RegisterWActionFactory("SetRelativeScale", [this](auto&& Attributes)
		{
			const std::string TargetName = Attributes.at("Target");
			WSceneComponent* TargetComp = GetWComponent<WSceneComponent>(TargetName);
			auto ScaleFunc = WExpressionParser::Bind<XMFLOAT3>(this, Attributes, "Scale", "{1, 1, 1}");
			return [TargetComp, ScaleFunc]()
			{
				TargetComp->SetRelativeScale(ScaleFunc());
			};
		});

	////////////////////////////////////////////////////////////////////////////////////////////////
	// WAction End
	////////////////////////////////////////////////////////////////////////////////////////////////

	////////////////////////////////////////////////////////////////////////////////////////////////
	// 
	// WEvent
	// 
	////////////////////////////////////////////////////////////////////////////////////////////////

	RegisterSystemEvent("OnSpawn", &mOnSpawnEvent);

	RegisterSystemEvent("OnDestroy", &mOnDestroyEvent);

	RegisterSystemEvent("OnTime", [this](auto&& Attributes) {
		TSharedPtr<FOnTimeEvent> OnTimeEvent = MakeShared<FOnTimeEvent>();
		ExtractAttribute(Attributes, "Time", OnTimeEvent->Time);

		mOnTimeEvents.push_back(std::move(OnTimeEvent));
		
		return &mOnTimeEvents.back()->Event;
		});

	RegisterSystemEvent("OnActivate", [this](const WAttributesMap& Attributes) -> WEvent* {
		auto Iter = Attributes.find("Target");

		WEvent* Event = nullptr;
		if (Iter != Attributes.end())
		{
			const std::string& Target = Iter->second;
			Event = GenerateWEvent(mOnActivateEventsMap, Target);
			
			if (WSceneComponent* Comp = GetWComponent(Target))
			{
				Comp->mOnActivate.AddLambda([Event]() {
					Event->Dispatch();
					});
			}
			else
			{
				ShowMessageBox("OnHit: Invalid target\n" + Target);
			}
		}
		else
		{
			ShowMessageBox("Event::OnActivate: Target attribute is required.");
			assert(false);
		}

		return Event;
		});

	RegisterSystemEvent("OnDeactivate", [this](const WAttributesMap& Attributes) -> WEvent* {
		auto Iter = Attributes.find("Target");

		WEvent* Event = nullptr;
		if (Iter != Attributes.end())
		{
			const std::string& Target = Iter->second;
			// 독립적인 WEvent 생성 및 맵 관리 (OnDeactivate용 맵 사용)
			Event = GenerateWEvent(mOnDeactivateEventsMap, Target);

			if (WSceneComponent* Comp = GetWComponent(Target))
			{
				// 컴포넌트의 비활성화 델리게이트에 바인딩
				Comp->mOnDeactivate.AddLambda([Event]() {
					if (Event) Event->Dispatch();
					});
			}
			else
			{
				ShowMessageBox("OnDeactivate: Invalid target\n" + Target);
			}
		}
		else
		{
			// OnDeactivate는 어떤 컴포넌트가 꺼질 때 발생할지 Target이 반드시 필요합니다.
			ShowMessageBox("Event::OnDeactivate: Target attribute is required.");
			assert(false);
		}

		return Event;
		});

	////////////////////////////////////////////////////////////////////////////////////////////////
	// WEvent End
	////////////////////////////////////////////////////////////////////////////////////////////////
}

void AActor::Tick(float DeltaSecond)
{
	Super::Tick(DeltaSecond);

	mElapsedTime += DeltaSecond;

	if (mLifeSpan > 0 && mElapsedTime >= mLifeSpan)
	{
		Destroy();
	}

	int NumOnTimeEvent = (int)mOnTimeEvents.size();
	while (mOnTimeEventIndex < NumOnTimeEvent && mOnTimeEvents[mOnTimeEventIndex]->Time <= mElapsedTime)
	{
		mOnTimeEvents[mOnTimeEventIndex++]->Event.Dispatch();
	}

	// mSplineFollowInfos는 std::vector 또는 std::list라고 가정합니다.
	for (auto it = mSplineFollowInfos.begin(); it != mSplineFollowInfos.end(); )
	{
		FSplineFollowInfo& Info = *it;

		// 1. 유효성 검사
		if (!Info.Target || !Info.Spline)
		{
			it = mSplineFollowInfos.erase(it);
			continue;
		}

		// 2. 시간 누적 및 비율 계산
		Info.ElapsedTime += DeltaSecond;
		float Alpha = Info.ElapsedTime / Info.Duration;

		// 3. 종료 및 루프 판정
		bool bFinished = false;
		if (Alpha >= 1.0f)
		{
			if (Info.bLoop)
			{
				// 루프 시 초과된 시간을 나머지 연산으로 넘겨주어 프레임 끊김 방지
				Info.ElapsedTime = fmodf(Info.ElapsedTime, Info.Duration);
				Alpha = Info.ElapsedTime / Info.Duration;
			}
			else
			{
				Alpha = 1.0f;
				bFinished = true;
			}
		}

		// 4. 스플라인 거리 기반 샘플링
		// 비율(Alpha)을 전체 길이(GetSplineLength)에 곱해 현재 가야 할 거리를 구합니다.
		float TotalLength = Info.Spline->GetSplineLength();
		float TargetDistance = TotalLength * Alpha;

		// 5. 트랜스폼 업데이트
		// GetWorldTransformAtDistanceAlongSpline을 사용하여 월드 좌표계 이동 지원
		FTransform NewTransform = Info.Spline->GetWorldTransformAtDistanceAlongSpline(TargetDistance);

		// 위치 적용
		Info.Target->SetWorldLocation(NewTransform.Translation);

		// 옵션에 따른 회전 적용
		if (Info.bUseRotation)
		{
			// FTransform 내에 XMFLOAT3 타입의 Rotation(Euler)이 있다고 가정하거나, 
			// 필요 시 Quaternion을 사용하여 회전을 직접 설정합니다.
			Info.Target->SetWorldRotation(NewTransform.Rotation);
		}

		// 6. 반복자 관리
		if (bFinished)
		{
			it = mSplineFollowInfos.erase(it);
		}
		else
		{
			++it;
		}
	}
}

void AActor::Destroy()
{
	if (!IsPendingKill())
	{
		GetWorld()->DestroyActor(GetWeakPtr<AActor>().lock());
		mOnDestroyEvent.Dispatch();
	}
}

void AActor::OnDestroy()
{
	Super::OnDestroy();

	for (auto Comp : mAllComponents)
	{
		Comp->OnDestroy();
	}
}

void AActor::OnActivate()
{
	Super::OnActivate();
	GetWorld()->ActivateActor(this);
	for (auto& Comp : mAllComponents)
	{
		if (Comp->IsActivate())
		{
			Comp->OnActivate();
		}
	}
}

void AActor::OnDeactivate()
{
	for (auto& Comp : mAllComponents)
	{
		Comp->OnDeactivate();
	}
	GetWorld()->DeactivateActor(this);
	Super::OnDeactivate();
}

void AActor::BeginPlay()
{
	if (IsActivate())
	{
		Activate();
		BeginComponents();
	}

	std::sort(mOnTimeEvents.begin(), mOnTimeEvents.end(), [](const TSharedPtr<FOnTimeEvent>& A, const TSharedPtr<FOnTimeEvent>& B) { return A->Time < B->Time; });

	mOnSpawnEvent.Dispatch();
}

XMFLOAT3 AActor::GetForwardVector() const
{
	XMMATRIX RotationMatrix = GetActorTransform().GetRotationMatrix();
	XMVECTOR L = XMVector3Transform({ 0.0f, 0.0f, 1.0f }, RotationMatrix);
	XMFLOAT3 Foward;
	XMStoreFloat3(&Foward, XMVector3Normalize(L));
	return Foward;
}

XMFLOAT3 AActor::GetRightVector() const
{
	XMMATRIX RotationMatrix = GetActorTransform().GetRotationMatrix();
	XMVECTOR R = XMVector3Transform({ 1.0f, 0.0f, 0.0f }, RotationMatrix);
	XMFLOAT3 Right;
	XMStoreFloat3(&Right, R);
	return Right;
}

XMFLOAT3 AActor::GetUpVector() const
{
	XMMATRIX RotationMatrix = GetActorTransform().GetRotationMatrix();
	XMVECTOR U = XMVector3Transform({ 0.0f, 1.0f, 0.0f }, RotationMatrix);
	XMFLOAT3 Up;
	XMStoreFloat3(&Up, U);
	return Up;
}

XMFLOAT4 AActor::GetActorQuaternion()
{
	return mRootComponent.lock()->GetWorldQuatRotation();
}

void AActor::SetRootComponent(TWeakPtr<WSceneComponent> Component)
{
	TSharedPtr<WSceneComponent> OldRoot = mRootComponent.lock();
	TSharedPtr<WSceneComponent> NewRoot = Component.lock();
	if (OldRoot && NewRoot)
	{
		if (OldRoot.get() == NewRoot.get())
		{
			return;
		}
		else
		{
			OldRoot->SetupAttachment(NewRoot.get());
		}
	}

	if (OldRoot)
	{
		OldRoot->PropagateWorldFloat4Dirty(true);
	}
	if (NewRoot)
	{
		NewRoot->PropagateWorldFloat4Dirty(true);
	}

	mRootComponent = Component;
}

void AActor::SetActorTransform(FTransform Transform)
{
	if (auto Root = mRootComponent.lock())
	{
		Root->SetRelativeTransform(Transform);
	}
}

void AActor::OnCreateComponent(WActorComponent* Comp)
{
	Comp->mOwner = this;
	if (WSceneComponent* SceneComp = dynamic_cast<WSceneComponent*>(Comp))
	{
		mAllSceneComponent.emplace_back(SceneComp->GetWeakPtr<WSceneComponent>());
		if (WPhysicsComponent* PhysicsComp = dynamic_cast<WPhysicsComponent*>(SceneComp))
		{
			mAllPhysicsComponents.emplace_back(PhysicsComp->GetWeakPtr<WPhysicsComponent>());
		}
	}
	else
	{
		mAllNoneSceneComponent.emplace_back(Comp->GetWeakPtr<WActorComponent>());
	}
}

void AActor::UpdateRecursive()
{
	if (auto Root = mRootComponent.lock())
	{
		Root->UpdateRecursive();
	}
}

void AActor::BeginComponents()
{
	for (int i = 0; i < mAllComponents.size(); ++i)
	{
		mAllComponents[i]->BeginComponent();
	}
}

void AActor::LoadBlueprint(const FBlueprintAsset* Blueprint)
{
	LoadWConfigs(Blueprint->mConfigs);

	WSceneComponent* RootComp = GetRootComponent();

	auto&& AttachedComp = Blueprint->mAttachedComponents;
	if (AttachedComp[0]->Attributes.at("Name") == "Root")
	{
		LoadWComponent_Internal(Blueprint->mAttachedComponents[0].get(), nullptr);

		if (AttachedComp.size() > 1)
		{
			std::wcout << L"Root 컴포넌트 지정시, Root의 하위에 있지 않은 컴포넌트는 무시됩니다." << std::endl;
		}
	}
	else
	{
		for (auto Comp : Blueprint->mAttachedComponents)
		{
			LoadWComponent_Internal(Comp.get(), RootComp);
		}

		WSceneComponent* Root = GetRootComponent();
		RegisterWComponent("Root", Root);
		RegisterWComponentCommonFunction("Root", Root);
	}

	LoadWEvents(Blueprint->mEvents, Blueprint->mCustomEvents);
}

WEvalValue AActor::ExecuteWFunction(const std::string& Name)
{
	return mWFunctionsMap[Name]();
}

void AActor::LoadWConfigs(const std::unordered_map<std::string, WAttributesMap>& Configs)
{
	if (Configs.count("General"))
	{
		const WAttributesMap& Attributes = Configs.at("General");

		ApplyAttribute<TArray<std::string>>(Attributes, "Tags", [=](auto&& Arry) {
			this->AddTags(Arry);
			});
	}

	if (Configs.count("LifeCycle"))
	{
		const WAttributesMap& Attributes = Configs.at("LifeCycle");

		ExtractAttribute(Attributes, "LifeSpan", mLifeSpan);
	}
}

void AActor::LoadWEvent(AActor::WEvent* Event, const TArray<TSharedPtr<FBlueprintActionNode>>& Actions)
{
	for (const auto& ActionInfo : Actions)
	{
		auto ActionFactory = mWActionFactoryMap.find(ActionInfo->Name);
		if (ActionFactory == mWActionFactoryMap.end())
		{
			ShowMessageBox(std::string("Invalid Action name:\n") + ActionInfo->Name);
			assert(false);
		}

		Event->AddAction(ActionFactory->second(ActionInfo->Attributes));
	}
}

void AActor::ApplyWComponentCommonAttribute(const WAttributesMap& Attributes, WSceneComponent* Comp)
{
	const std::string Name = Attributes.at("Name");

	ApplyAttribute<XMFLOAT3>(Attributes, "Loc", [&](const XMFLOAT3& v) {
		Comp->SetRelativeLocation(v);
		});

	ApplyAttribute<XMFLOAT3>(Attributes, "Rot", [&](const XMFLOAT3& v) {
		// 실수 방지: Rotation 전용 세터 호출
		Comp->SetRelativeRotation(v);
		});

	ApplyAttribute<XMFLOAT3>(Attributes, "Scale", [&](const XMFLOAT3& v) {
		// 실수 방지: Scale 전용 세터 호출
		Comp->SetRelativeScale(v);
		});

	ApplyAttribute<bool>(Attributes, "Activate", true, [&](const bool& v) {
		if (!v)
		{
			Comp->Deactivate();
		}
		});
}

void AActor::RegisterWComponentCommonFunction(const std::string Name, WSceneComponent* Comp)
{
	RegisterWFunction(Name + ".GetWorldLocation()", [Comp]()
		{
			return Comp->GetWorldLocation();
		});

	RegisterWFunction(Name + ".GetWorldRotation()", [Comp]()
		{
			return Comp->GetWorldRotation();
		});

	RegisterWFunction(Name + ".GetWorldScale()", [Comp]()
		{
			return Comp->GetWorldScale();
		});

	RegisterWFunction(Name + ".GetRelativeLocation()", [Comp]()
		{
			return Comp->GetRelativeLocation();
		});

	RegisterWFunction(Name + ".GetRelativeRotation()", [Comp]()
		{
			return Comp->GetRelativeRotation();
		});

	RegisterWFunction(Name + ".GetRelativeScale()", [Comp]()
		{
			return Comp->GetRelativeScale();
		});
}

void AActor::RegisterWComponentFactory(const std::string& Type, WComponentFactory Lambda)
{
	assert(mWComponentFactoryMap.count(Type) == 0 && "Already registered component type");

	mWComponentFactoryMap[Type] = Lambda;
}

void AActor::RegisterWActionFactory(const std::string Name, WActionFactoryFunc Lambda)
{
	mWActionFactoryMap[Name] = Lambda;
}

void AActor::RegisterSystemEvent(const std::string& Name, WEvent* Event)
{
	RegisterSystemEvent(Name, [=](const WAttributesMap&) { return Event; });
}

void AActor::RegisterSystemEvent(const std::string& Name, WEventLoader Loader)
{
	mSystemEventLoaders[Name] = Loader;
}

void AActor::RegisterWFunction(const std::string& Name, WFunction Lambda)
{
	if (mWFunctionsMap.count(Name) > 0)
	{
		ShowMessageBox(L"이미 등록된 WFunction 입니다.");
		assert(false);
	}

	mWFunctionsMap[Name] = std::move(Lambda);
}

void AActor::RegisterWProperty(const std::string& Name, WEvalValue& Value)
{
	std::visit([this, Name](auto&& v) 
		{
			using T = std::decay_t<decltype(v)>;
			RegisterWProperty<T>(Name,&v);
		}, Value);
}

AActor::WEvent* AActor::GenerateWEvent(std::unordered_map<std::string, TSharedPtr<WEvent>>& Container, const std::string& Name)
{
	if (Container.count(Name) > 0)
	{
		ShowMessageBox(L"이미 등록된 Target 이벤트입니다.");
		assert(false);
	}

	Container[Name] = MakeShared<WEvent>();
	return Container[Name].get();
}

AActor::WEvent* AActor::GenerateWEvent(TArray<TSharedPtr<WEvent>>& Container)
{
	Container.push_back(MakeShared<WEvent>());
	return Container.back().get();
}

void AActor::SetWProperty(const std::string& Name, WEvalValue Value)
{
	WSourceRef Property = mWPropertiesMap[Name];

	// 실행 시점에는 단순 값 대입만 발생 (파싱 X, 매우 빠름)
	std::visit([](auto&& TargetPtr, auto&& SourceValue) {
		using TargetType = std::remove_pointer_t<std::decay_t<decltype(TargetPtr)>>;
		using SourceType = std::decay_t<decltype(SourceValue)>;

		if constexpr (std::is_same_v<TargetType, SourceType>) {
			if (TargetPtr) *TargetPtr = SourceValue;
		}
		}, Property, Value);
}

void AActor::LoadWComponent_Internal(FBlueprintComponentNode* CompNode, WSceneComponent* Parent)
{
	assert(mWComponentFactoryMap.count(CompNode->Type) > 0 && "Unregistered component class");

	WSceneComponent* Comp = mWComponentFactoryMap[CompNode->Type](CompNode->Attributes);
	assert(Comp);
	if (Parent)
	{
		Comp->SetupAttachment(Parent);
	}
	else
	{
		SetRootComponent(Comp->GetWeakPtr<WSceneComponent>());
	}

	// SceneComponent 공용 속성 처리
	const std::string& Name = CompNode->Attributes.at("Name");
	ApplyWComponentCommonAttribute(CompNode->Attributes, Comp);
	RegisterWComponentCommonFunction(Name, Comp);

	if (CompNode->Attributes.count("Name") > 0) RegisterWComponent(CompNode->Attributes["Name"], Comp);
	
	for (const auto ChildCompNode : CompNode->AttachedComponents)
	{
		LoadWComponent_Internal(ChildCompNode.get(), Comp);
	}
}

void AActor::LoadWEvents(
	const TArray<TSharedPtr<FBlueprintEventNode>>& SystemEvents,
	const TArray<TSharedPtr<FBlueprintEventNode>>& CustomEvents
)
{
	// 1. 시스템 이벤트 처리 (중복 허용, 로더가 바인딩 전략 결정)
	for (const auto& EventNode : SystemEvents)
	{
		auto loaderIt = mSystemEventLoaders.find(EventNode->Name);
		if (loaderIt != mSystemEventLoaders.end())
		{
			// 각 노드마다 독립적인 WEvent 인스턴스가 생성되도록 로더 내부에 설계
			WEvent* Event = loaderIt->second(EventNode->Attributes);
			LoadWEvent(Event, EventNode->Actions);
		}
		else
		{
			ShowMessageBox("Unknown System Event: " + EventNode->Name);
		}
	}

	// 2. 커스텀 이벤트 처리 (이름 기반 호출을 위해 맵에 등록)
	for (const auto& EventNode : CustomEvents)
	{
		auto NewCustomEvent = std::make_shared<WEvent>();

		// 액션들 로드
		LoadWEvent(NewCustomEvent.get(), EventNode->Actions);

		// 맵에 저장 (커스텀 이벤트는 이름이 고유해야 함)
		mCustomEventsMap[EventNode->Name] = NewCustomEvent;
	}
}

void AActor::RegisterWComponent(const std::string& Name, WSceneComponent* Comp)
{
	assert(mWComponentsMap.count(Name) == 0 && L"중복된 컴포넌트 이름 입니다.");

	mWComponentsMap[Name] = Comp;
}