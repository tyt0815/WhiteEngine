#pragma once
#include "Utility/Class.h"
#include <functional>
#include <unordered_map>
#include <string>
#include "Utility/Memory.h"

class AActor; // 전방 선언

// Actor 생성 함수 정의
using ActorCreator = std::function<std::shared_ptr<AActor>()>;

class FActorFactory {
public:
    SINGLETON(FActorFactory);

private:
    // 실제 객체를 생성하는 내부 로직
    std::shared_ptr<AActor> CreateActorInternal(const std::string& Name);

    std::shared_ptr<AActor> CreateBlueprintActor_Internal(const std::wstring& BlueprintName);

    std::unordered_map<std::string, ActorCreator> mRegistry;

public:
    // 컴포넌트 팩토리와 동일한 등록 인터페이스
    void RegisterActor(const std::string& Name, ActorCreator Creator);

    // 외부에서 문자열로 액터를 찍어내는 정적 함수
    template<typename T>
    __forceinline static std::shared_ptr<T> CreateBlueprintActor(const std::wstring& BlueprintName)
    {
        return Cast<T>(GetInstance()->CreateBlueprintActor_Internal(BlueprintName));
    }
};

#define REGISTER_ACTOR(ClassName) \
    inline static bool ClassName##_Registered = []() { \
        FActorFactory::GetInstance()->RegisterActor(#ClassName, []() { \
            return std::make_shared<ClassName>(); \
        }); \
        return true; \
    }()