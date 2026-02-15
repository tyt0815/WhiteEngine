#pragma once

#include <string>
#include <unordered_map>
#include <functional>
#include <memory>
#include "Utility/Class.h"

class AActor;
class WActorComponent;
using WAttributesMap = std::unordered_map<std::string, std::string>;

// 컴포넌트 생성 함수 정의: (소유자 액터, 속성 맵) -> 생성된 컴포넌트 포인터
using WComponentCreator = std::function<WActorComponent* (AActor*, const WAttributesMap&)>;

class WComponentRegistry
{
    SINGLETON(WComponentRegistry);

public:
    __forceinline static void Register(const std::string& Tag, WComponentCreator Creator)
    {
        GetInstance()->Register_Internal(Tag, Creator);
    }

    __forceinline static WActorComponent* Create(const std::string& Tag, AActor* Owner, const WAttributesMap& Attributes)
    {
        return GetInstance()->Create_Internal(Tag, Owner, Attributes);
    }

private:
    void Register_Internal(const std::string& Tag, WComponentCreator Creator);

    WActorComponent* Create_Internal(const std::string& Tag, AActor* Owner, const WAttributesMap& Attributes);

    std::unordered_map<std::string, WComponentCreator> mCreators;
};