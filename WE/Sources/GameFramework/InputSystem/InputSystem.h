#pragma once

#include <comdef.h>

enum class EInputType
{
	EIT_MouseDown,
	EIT_MouseUp,
	EIT_MouseMove,
	EIT_MouseWheel,
	EIT_KeyDown,
	EIT_KeyUp,
	EIT_None
};

inline bool IsKeyDown(int vkeyCode)
{
	return (GetAsyncKeyState(vkeyCode) & 0x8000) != 0;
}

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