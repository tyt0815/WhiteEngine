#pragma once

#include <string>
#include <unordered_map>
#include <functional>
#include "Utility/Class.h"
#include "Utility/Memory.h"

class WObject;
using WAttributesMap = std::unordered_map<std::string, std::string>;

class WEvent
{
public:
	static WEvent* GenerateTargetEvent(std::unordered_map<std::string, TSharedPtr<WEvent>>& Container, const std::string& Target);

	static WEvent* GenerateEvent(std::vector<TSharedPtr<WEvent>>& Container);

	void Dispatch() const
	{
		for (const auto& Action : mActions) Action();
	}
	void AddAction(const std::function<void()>& Action)
	{
		mActions.push_back(Action);
	}

private:
	std::vector<std::function<void()>> mActions;
};

// 이벤트 생성 함수: (대상 객체, 속성) -> 관리되는 WEvent 포인터 반환
using WEventCreator = std::function<WEvent* (WObject*, const WAttributesMap&)>;

class WEventRegistry
{
    SINGLETON(WEventRegistry)
public:

    // 시스템 이벤트 등록 (생성자에서 호출)
    void Register(const std::string& Tag, WEventCreator Creator);

    // XML 태그를 보고 이벤트를 생성/연결하고 핸들을 반환
    WEvent* Create(const std::string& Tag, WObject* Target, const WAttributesMap& Attributes);

private:
    std::unordered_map<std::string, WEventCreator> mCreators;
};