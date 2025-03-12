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
	EMIT_LMove,
	EMIT_RMove,
	EMIT_MMove,
	EMIT_Move,
	EMIT_Wheel,
	EMIT_None
};

enum EKeyboardInputType
{
	EKIT_Down,
	EKIT_Up,
	EKIT_None,
};

enum EInputType
{
	EIT_MouseDown,
	EIT_MouseUp,
	EIT_MouseMove,
	EIT_MouseWheel,
	EIT_KeyDown,
	EIT_KeyUp,
	EIT_None
};

class FMouseInputParameter
{
public:
	FMouseInputParameter() = default;

private:
	// 내부 함수: 상위 비트와 하위 비트를 설정하는 공통 로직
	inline void SetLowBits(unsigned long long& Target, unsigned int Value)
	{
		Target = (Target & 0xffffffff00000000ull) | Value;
	}

	inline void SetHighBits(unsigned long long& Target, unsigned int Value)
	{
		Target = (Target & 0x00000000ffffffffull) | (static_cast<unsigned long long>(Value) << 32);
	}

	// 내부 함수: 상위 비트와 하위 비트를 가져오는 공통 로직
	inline unsigned int GetLowBits(unsigned long long Target) const
	{
		return static_cast<unsigned int>(Target & 0xffffffffull);
	}

	inline unsigned int GetHighBits(unsigned long long Target) const
	{
		return static_cast<unsigned int>((Target >> 32) & 0xffffffffull);
	}

	unsigned long long X = 0;
	unsigned long long Y = 0;

public:
	inline void SetX(unsigned int Value)
	{
		SetLowBits(X, Value);
	}
	inline void SetLastX(unsigned int Value)
	{
		SetHighBits(X, Value);
	}
	inline void SetY(unsigned int Value)
	{
		SetLowBits(Y, Value);
	}
	inline void SetLastY(unsigned int Value)
	{
		SetHighBits(Y, Value);
	}
	inline unsigned int GetX() const
	{
		return GetLowBits(X);
	}
	inline unsigned int GetLastX() const
	{
		return GetHighBits(X);
	}
	inline unsigned int GetY() const
	{
		return GetLowBits(Y);
	}
	inline unsigned int GetLastY() const
	{
		return GetHighBits(Y);
	}
	inline void SetParameters(unsigned int X, unsigned int Y, unsigned int LastX, unsigned int LastY)
	{
		SetX(X);
		SetY(Y);
		SetLastX(LastX);
		SetLastY(LastY);
	}
};

class FInputSystemManager
{
	SINGLETON(FInputSystemManager);
public:
	void Tick();
	template<typename T>
	void BindMouseAction(EMouseInputType Type, T* ObjectPtr, void(T::*ActionFunction)(FMouseInputParameter));
	template<typename T>
	void BindKeyboardAction(char Key, T* ObjectPtr, void(T::* ActionFunction)());
private:
	void ProcessKeyboardActions();
	void Test();
	std::vector<std::vector<std::function<void(FMouseInputParameter)>>> mMouseActions;
	std::vector<std::function<void()>> mKeyboardActions;

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
inline void FInputSystemManager::BindMouseAction(EMouseInputType Type, T* ObjectPtr, void(T::* ActionFunction)(FMouseInputParameter))
{
}

template<typename T>
inline void FInputSystemManager::BindKeyboardAction(char Key, T* ObjectPtr, void(T::* ActionFunction)())
{
	Key = toupper(Key);
	std::function<void()> BoundFunction = std::bind(ActionFunction, ObjectPtr);
	mKeyboardActions.push_back([=]()
		{
			if (IsKeyDown(Key))
			{
				BoundFunction();
			}
		}
	);
}
