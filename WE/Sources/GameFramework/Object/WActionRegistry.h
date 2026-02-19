#pragma once

#include <string>
#include <unordered_map>
#include <functional>
#include <vector>
#include "Utility/Class.h"

class WObject;
using WAttributesMap = std::unordered_map<std::string, std::string>;
// 액션 실행 함수: 인자 없이 실행되는 클로저
using WActionLambda = std::function<void()>;
using WActionFactory = std::function<WActionLambda(WObject* Target)>;
// 팩토리 등록 함수: (대상 객체, 속성 맵) -> 실행 람다 반환
using WActionCreator = std::function<WActionLambda(WObject*, const WAttributesMap&, const std::vector<WActionFactory>&)>;

class WActionRegistry
{
    SINGLETON(WActionRegistry);
public:
    __forceinline static void Register(const std::string& Tag, WActionCreator Creator)
    {
        return GetInstance()->Register_Internal(Tag, Creator);
    }

    __forceinline static WActionLambda Create(const std::string& Tag, WObject* Target, const WAttributesMap& Attributes, const std::vector<WActionFactory>& SubActionFactories)
    {
        return GetInstance()->Create_Internal(Tag, Target, Attributes, SubActionFactories);
    }
    

private:
    void Register_Internal(const std::string& Tag, WActionCreator Creator);

    WActionLambda Create_Internal(const std::string& Tag, WObject* Target, const WAttributesMap& Attributes, const std::vector<WActionFactory>& SubActionFactories);

    std::unordered_map<std::string, WActionCreator> mCreators;
};