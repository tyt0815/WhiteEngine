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
    std::shared_ptr<AActor> CreateActor_Internal(const std::string& Name);

    // 컴포넌트 팩토리와 동일한 등록 인터페이스
    void RegisterActor_Internal(const std::string& Name, ActorCreator Creator);

    std::unordered_map<std::string, ActorCreator> mRegistry;

public:
    template<typename T>
    __forceinline static TSharedPtr<T> CreateActor(const std::string& Name)
    {
        return Cast<T>(GetInstance()->CreateActor_Internal(Name));
    }

    __forceinline static void RegisterActor(const std::string& Name, ActorCreator Creator)
    {
        GetInstance()->RegisterActor_Internal(Name, Creator);
    }
};

#define REGISTER_ACTOR(ClassName) \
    inline static bool ClassName##_Registered = []() { \
        FActorFactory::RegisterActor(#ClassName, []() { \
            return std::make_shared<ClassName>(); \
        }); \
        return true; \
    }();