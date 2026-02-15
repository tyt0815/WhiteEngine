#include "WEventRegistry.h"
#include "Actor/Actor.h"
#include "Utility/Debug.h"

WEvent* WEvent::GenerateTargetEvent(std::unordered_map<std::string, TSharedPtr<WEvent>>& Container, const std::string& Target)
{
    if (Container.count(Target) > 0)
    {
        std::wcout << L"이미 등록된 이벤트 입니다." << std::endl;
        assert(false);
    }

    Container[Target] = MakeShared<WEvent>();
    return Container[Target].get();
}

WEvent* WEvent::GenerateEvent(std::vector<TSharedPtr<WEvent>>& Container)
{
    Container.push_back(MakeShared<WEvent>());
    return Container.back().get();
}


WEventRegistry::WEventRegistry()
{
    //Register("OnSpawn", [](WObject* Target, const WAttributesMap& Attr) -> WEvent* {
    //    AActor* Owner = static_cast<AActor*>(Target);
    //    return Owner->GetOnSpawnEvent(); // 액터가 기본으로 가진 mOnSpawnEvent 반환
    //    });

    //Register("OnDestroy", [](WObject* Target, const WAttributesMap& Attr) -> WEvent* {
    //    AActor* Owner = static_cast<AActor*>(Target);
    //    return Owner->GetOnDestroyEvent(); // mOnDestroyEvent 반환
    //    });

    //// 2. OnActivate / OnDeactivate: 특정 컴포넌트의 활성화 감시
    //auto RegisterComponentStateEvent = [&](const std::string& Tag, bool bActivateEvent) {
    //    Register(Tag, [=](WObject* Target, const WAttributesMap& Attr) -> WEvent* {
    //        AActor* Owner = static_cast<AActor*>(Target);
    //        auto Iter = Attr.find("Target");

    //        if (Iter == Attr.end()) {
    //            ShowMessageBox(Tag + ": Target attribute is required.");
    //            return nullptr;
    //        }

    //        const std::string& TargetName = Iter->second;
    //        WSceneComponent* Comp = Owner->GetWComponent<WSceneComponent>(TargetName);

    //        if (!Comp) {
    //            ShowMessageBox(Tag + ": Invalid target\n" + TargetName);
    //            return nullptr;
    //        }

    //        // 액터 내부 맵(mOnActivateEventsMap 등)에서 관리되는 WEvent 생성
    //        // bActivateEvent에 따라 다른 맵을 사용하도록 AActor 인터페이스 설계 권장
    //        WEvent* Event = Owner->GenerateComponentStateEvent(Tag, TargetName);

    //        if (bActivateEvent) {
    //            Comp->mOnActivate.AddLambda([Event]() { if (Event) Event->Dispatch(); });
    //        }
    //        else {
    //            Comp->mOnDeactivate.AddLambda([Event]() { if (Event) Event->Dispatch(); });
    //        }

    //        return Event;
    //        });
    //};

    //RegisterComponentStateEvent("OnActivate", true);
    //RegisterComponentStateEvent("OnDeactivate", false);


    //// 1. OnHit: 충돌 발생 시 트리거
    //Register("OnHit", [](WObject* Target, const WAttributesMap& Attr) -> WEvent* {
    //    AActor* Owner = static_cast<AActor*>(Target);
    //    auto it = Attr.find("Target");

    //    if (it != Attr.end()) {
    //        const std::string& TargetName = it->second;
    //        auto* CollisionGen = dynamic_cast<FCollisionGeneratorBase*>(Owner->GetWComponent(TargetName));
    //        if (!CollisionGen) return nullptr;

    //        // 액터 인스턴스 내부에 이벤트를 하나 생성해서 반환
    //        WEvent* NewEvent = Owner->GenerateWEvent();
    //        auto FilterFunc = WExpressionParser::Bind<TArray<std::string>>(Target, Attr, "Filter", "[]");

    //        CollisionGen->mOnCollision.AddLambda([Owner, NewEvent, FilterFunc](auto* Inst, auto* HitComp, XMFLOAT3 Pos, auto... args) {
    //            Owner->SetInternalImpactPoint(Pos); // 충돌 지점 저장
    //            auto Filter = FilterFunc();

    //            if (Filter.size() == 0) {
    //                NewEvent->Dispatch();
    //            }
    //            else {
    //                for (const auto& Tag : Filter) {
    //                    if (HitComp->HasTag(Tag, true)) { NewEvent->Dispatch(); break; }
    //                }
    //            }
    //            });
    //        return NewEvent;
    //    }
    //    return Owner->GetCommonOnHitEvent();
    //    });

    //// 2. 투사체 관련 이벤트 (OnLockon, OnBounce 등)
    //auto RegisterProjEvent = [&](const std::string& Tag, auto DelegatePtr) {
    //    Register(Tag, [=](WObject* Target, const WAttributesMap& Attr) -> WEvent* {
    //        AActor* Owner = static_cast<AActor*>(Target);
    //        auto* Comp = Owner->GetWComponent<WProjectileMovementComponent>(Attr.at("Target"));
    //        if (!Comp) return nullptr;

    //        WEvent* Event = Owner->GenerateWEvent();
    //        (Comp->*DelegatePtr).AddLambda([Event]() { Event->Dispatch(); });
    //        return Event;
    //        });
    //};

    //RegisterProjEvent("OnLockon", &WProjectileMovementComponent::mOnLockon);
    //RegisterProjEvent("OnBounce", &WProjectileMovementComponent::mOnBounce);
    //RegisterProjEvent("OnHomingFail", &WProjectileMovementComponent::mOnHomingFail);

    //// 3. OnAnimStop
    //Register("OnAnimStop", [](WObject* Target, const WAttributesMap& Attr) -> WEvent* {
    //    AActor* Owner = static_cast<AActor*>(Target);
    //    auto* Comp = Owner->GetWComponent<WObjectAnimComponent>(Attr.at("Target"));
    //    if (!Comp) return nullptr;

    //    WEvent* Event = Owner->GenerateWEvent();
    //    Comp->mOnStop.AddLambda([Event]() { Event->Dispatch(); });
    //    return Event;
    //    });
}

WEventRegistry::~WEventRegistry()
{
}

void WEventRegistry::Register(const std::string& Tag, WEventCreator Creator)
{
    mCreators[Tag] = std::move(Creator);
}

WEvent* WEventRegistry::Create(const std::string& Tag, WObject* Target, const WAttributesMap& Attributes)
{
    auto it = mCreators.find(Tag);
    if (it != mCreators.end())
    {
        return it->second(Target, Attributes);
    }
    return nullptr;
}