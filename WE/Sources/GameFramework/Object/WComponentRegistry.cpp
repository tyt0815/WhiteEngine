#include "WComponentRegistry.h"
#include "Actor/Actor.h"
#include "Parser.h"
#include "Component/ActorComponent.h"
#include "Component/StaticMeshComponent.h"
#include "Component/ProjectileMovementComponent.h"
#include "Component/ObjectAnimComponent.h"
#include "Component/SplineComponent.h"
#include "Component/BoxCollisionComponent.h"
#include "Component/SphereCollisionComponent.h"
#include "Component/CapsuleCollisionComponent.h"
#include "Component/SplineCollisionComponent.h"

WComponentRegistry::WComponentRegistry()
{
    Register_Internal("Dummy", [](AActor* Owner, const WAttributesMap& Attr) -> WActorComponent* {
        // 특별한 기능 없이 트랜스폼만 가지는 SceneComponent 생성
        return Owner->CreateComponent<WSceneComponent>();
        });

    Register_Internal("Mesh", [](AActor* Owner, const WAttributesMap& Attr) -> WActorComponent* {
        WStaticMeshComponent* Comp = Owner->CreateComponent<WStaticMeshComponent>();

        // Static Mesh 에셋 적용
        auto AssetIter = Attr.find("Asset");
        if (AssetIter != Attr.end())
        {
            Comp->SetStaticMesh(AssetIter->second);
        }

        return Comp;
        });

    Register_Internal("Spline", [](AActor* Owner, const WAttributesMap& Attr) -> WActorComponent* {
        WSplineComponent* Comp = Owner->CreateComponent<WSplineComponent>();

        // 스플라인 에셋 로드
        auto AssetIter = Attr.find("Asset");
        if (AssetIter != Attr.end())
        {
            Comp->LoadSplineFromAsset(AssetIter->second);
        }

        return Comp;
        });

    Register_Internal("Collision", [](AActor* Owner, const WAttributesMap& Attr) -> WActorComponent* {
        WSceneComponent* Component = nullptr;
        const std::string& Name = Attr.at("Name");
        const std::string& Type = Attr.at("Type");

        // 타입별 생성
        if (Type == "Spline") {
            auto* Comp = Owner->CreateComponent<WSplineCollisionComponent>();
            if (Attr.count("Asset")) Comp->LoadSplineFromAsset(Attr.at("Asset"));

            float Segment = 1.0f;
            if (ExtractAttribute(Attr, "Segment", Segment)) Comp->SetSegment((int)Segment);

            bool bUseBB = true;
            if (ExtractAttribute(Attr, "BoundingBox", bUseBB)) Comp->SetBoundingBox(bUseBB);
            Component = Comp;
        }
        else if (Type == "Box") {
            auto* Comp = Owner->CreateComponent<WBoxCollisionComponent>();
            ApplyAttribute<XMFLOAT3>(Attr, "Extent", [Comp](auto& v) { Comp->SetExtent(v); });
            Component = Comp;
        }
        else if (Type == "Capsule") {
            auto* Comp = Owner->CreateComponent<WCapsuleCollisionComponent>();
            float R = 1.0f, H = 1.0f;
            ExtractAttribute(Attr, "Radius", R);
            ExtractAttribute(Attr, "HalfHeight", H);
            Comp->SetCapsuleSize(R, H);
            Component = Comp;
        }
        else if (Type == "Sphere") {
            auto* Comp = Owner->CreateComponent<WSphereCollisionComponent>();
            ApplyAttribute<float>(Attr, "Radius", [Comp](float v) { Comp->SetRadius(v); });
            Component = Comp;
        }

        // 공통 Collision 설정 (FCollisionGeneratorBase 인터페이스)
        if (auto* Gen = dynamic_cast<FCollisionGeneratorBase*>(Component)) {
            Gen->GenerateCollision();

            // 속성 적용 및 프로퍼티 등록 (Name.Property 형식)
            ApplyAttribute<float>(Attr, "Delay", 0.0f, [Owner, Gen, Name](float v) {
                Gen->SetHitDelay(v);
                Owner->RegisterWProperty(Name + ".Delay", &Gen->mHitDelay);
                });

            ApplyAttribute<float>(Attr, "MaxHit", 0.0f, [Owner, Gen, Name](float v) {
                Gen->SetMaxHit((int)v);
                Owner->RegisterWProperty(Name + ".MaxHit", &Gen->mMaxHit);
                });

            ApplyAttribute<float>(Attr, "Damage", 0.0f, [Owner, Gen, Name](float v) {
                Gen->SetDamage(v);
                Owner->RegisterWProperty(Name + ".Damage", &Gen->mDamage);
                });

            ApplyAttribute<std::set<std::string>>(Attr, "TargetTags", [Owner, Gen, Name](auto& v) {
                Gen->AddTargetTags(v);
                Owner->RegisterWProperty(Name + ".TargetTags", &Gen->mTargetTags);
                });

            ApplyAttribute<bool>(Attr, "Debug", false, [Owner, Gen, Name](bool v) {
                Gen->SetDebug(v);
                Owner->RegisterWProperty(Name + ".Debug", &Gen->mbDebug);
                });

            ApplyAttribute<float>(Attr, "Interval", 1.0f / 60.0f, [Gen](float v) { Gen->mCollisionInterval = v; });
        }
        return Component;
        });

    // 5. Movement Component
    Register_Internal("Movement", [](AActor* Owner, const WAttributesMap& Attr) -> WActorComponent* {
        const std::string& Name = Attr.at("Name");
        auto* Comp = Owner->CreateComponent<WProjectileMovementComponent>();

        // 물리/이동 속성
        ApplyAttribute<XMFLOAT3>(Attr, "InitSpeed", [Comp](auto& v) { Comp->SetInitialVelocity(v); });

        auto RegProp = [&](const std::string& PropName, auto* TargetPtr) {
            Owner->RegisterWProperty(Name + "." + PropName, TargetPtr);
        };

        ApplyAttribute<float>(Attr, "MaxSpeed", [Comp, RegProp](float v) {
            Comp->SetMaxSpeed(v); RegProp("MaxSpeed", &Comp->mMaxSpeed);
            });

        ApplyAttribute<float>(Attr, "Acceleration", [Comp, RegProp](float v) {
            Comp->SetAcceleration(v); RegProp("Acceleration", &Comp->mAcceleration);
            });

        // 호밍 (Homing)
        ApplyAttribute<std::string>(Attr, "HomingStrategy", "None", [Comp, RegProp](auto& v) {
            Comp->mHomingStrategy = v; RegProp("HomingStrategy", &Comp->mHomingStrategy);
            });

        ApplyAttribute<std::set<std::string>>(Attr, "HomingTags", [Comp, RegProp](auto& v) {
            Comp->mHomingTargetTags = v; RegProp("HomingTags", &Comp->mHomingTargetTags);
            });

        ApplyAttribute<float>(Attr, "HomingTurnRate", [Comp, RegProp](float v) {
            Comp->SetHomingTurnLimit(v); RegProp("HomingTurnRate", &Comp->mHomingTurnLimit);
            });

        // 바운스 (Bounce)
        ApplyAttribute<bool>(Attr, "ShouldBounce", false, [Comp, RegProp](bool v) {
            Comp->mShouldBounce = v; RegProp("ShouldBounce", &Comp->mShouldBounce);
            });

        // 웨이포인트 (Waypoint)
        ApplyAttribute<std::vector<XMFLOAT3>>(Attr, "Waypoints", [Comp, RegProp](auto& v) {
            Comp->mConfigWaypoints = v; RegProp("Waypoints", &Comp->mConfigWaypoints);
            });

        return Comp;
        });

    // 6. Anim Component
    Register_Internal("Anim", [](AActor* Owner, const WAttributesMap& Attr) -> WActorComponent* {
        return Owner->CreateComponent<WObjectAnimComponent>();
        });

}

WComponentRegistry::~WComponentRegistry()
{
}

void WComponentRegistry::Register_Internal(const std::string& Tag, WComponentCreator Creator)
{
    mCreators[Tag] = std::move(Creator);
}

WActorComponent* WComponentRegistry::Create_Internal(const std::string& Tag, AActor* Owner, const WAttributesMap& Attributes)
{
    auto it = mCreators.find(Tag);
    if (it != mCreators.end())
    {
        // 등록된 생성 함수 호출
        return it->second(Owner, Attributes);
    }

    // 등록되지 않은 태그일 경우 에러 로그 출력 권장
    return nullptr;
}