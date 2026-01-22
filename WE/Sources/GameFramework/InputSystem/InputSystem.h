#pragma once
#include <comdef.h>
#include <functional>
#include <vector>
#include "Utility/Class.h"

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
	void BindKeyboardAction(char Key, T* ObjectPtr, void(T::* ActionFunction)(float));
	void ProcessMouseInput(EMouseInputType Type, int X, int Y);
private:
	void ProcessKeyboardActions(float Delta);
	std::vector<std::vector<std::function<void(FMouseInputParameter)>>> mMouseActions;
	std::vector<std::function<void(float)>> mKeyboardActions;

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
inline void FInputSystemManager::BindKeyboardAction(char Key, T* ObjectPtr, void(T::* ActionFunction)(float))
{
	Key = toupper(Key);
	std::function<void(float)> BoundFunction = std::bind(ActionFunction, ObjectPtr, std::placeholders::_1);
	mKeyboardActions.push_back([=](float Delta)
		{
			if (IsKeyDown(Key))
			{
				BoundFunction(Delta);
			}
		}
	);
}
