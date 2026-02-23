#pragma once
#include <comdef.h>
#include <functional>
#include <vector>
#include <unordered_map>
#include <array>
#include "Utility/Class.h"

namespace EKeyboardInputType
{
	enum Type
	{
		EKIT_Pressed = 0,		// 누른 찰나
		EKIT_Down,				// 누르는 중
		EKIT_Released,			// 뗀 찰나
		EKIT_None
	};
}

enum EMouseInputType
{
	EMIT_LDown,
	EMIT_RDown,
	EMIT_MDown,
	EMIT_LUp,
	EMIT_RUp,
	EMIT_MUp,
	EMIT_Move,
	EMIT_Wheel,
	EMIT_None
};

struct FMouseInputParameter
{
	int X;
	int Y;
};

class FInputSystemManager
{
	SINGLETON(FInputSystemManager);
public:
	void Tick(float Delta);
	template<typename T>
	void BindMouseAction(EMouseInputType Type, T* ObjectPtr, void(T::*ActionFunction)(FMouseInputParameter));
	template<typename T>
	void BindKeyboardAction(char Key, EKeyboardInputType::Type InputType, T* ObjectPtr, void(T::* ActionFunction)(float));
	void ProcessMouseInput(EMouseInputType Type, int X, int Y);
private:
	void ProcessKeyboardActions(float Delta);

	void UpdateKeyStates();

	std::vector<std::vector<std::function<void(FMouseInputParameter)>>> mMouseActions;
	std::vector<std::function<void(float)>> mKeyboardActions;

	std::unordered_map<char, EKeyboardInputType::Type> mKeyStates;

public:
	inline bool IsKeyDown(int VKeyCode)
	{
		return (GetAsyncKeyState(VKeyCode) & 0x8000) != 0;
	}
};

inline FInputSystemManager* GetInputSystemManager()
{
	return FInputSystemManager::GetInstance();
}

template<typename T>
inline void FInputSystemManager::BindMouseAction(EMouseInputType Type, T* ObjectPtr, void(T::* ActionFunction)(FMouseInputParameter a))
{
	std::function<void(FMouseInputParameter a)> BoundFunction = std::bind(ActionFunction, ObjectPtr, std::placeholders::_1);
	mMouseActions[Type].push_back(BoundFunction);
}

template<typename T>
inline void FInputSystemManager::BindKeyboardAction(char Key, EKeyboardInputType::Type InputType, T* ObjectPtr, void(T::* ActionFunction)(float))
{
	Key = toupper(Key);
	std::function<void(float)> BoundFunction = std::bind(ActionFunction, ObjectPtr, std::placeholders::_1);

	// 액션에 등록할 때, 해당 키가 상태 추적 대상임을 등록
	if (mKeyStates.count(Key) == 0)
	{
		mKeyStates[Key] = EKeyboardInputType::EKIT_None;
	}

	// 특정 타입(Pressed/Down/Released) 벡터에 람다 등록
	mKeyboardActions.push_back([=](float Delta)
		{
			if (mKeyStates[Key] == InputType)
			{
				BoundFunction(Delta);
			}
		}
	);
}
