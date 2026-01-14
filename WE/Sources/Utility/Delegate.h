#pragma once
#include <functional>

class FDelegate
{
public:
	template<typename T>
	void Bind(T* ObjectPtr, void(T::* ActionFunction)())
	{
		mBoundFunction = std::bind(ActionFunction, ObjectPtr);
	}

	__forceinline void Execute()
	{
		if (mBoundFunction)
		{
			mBoundFunction();
		}
	}

private:
	std::function<void()> mBoundFunction;
};

class FMulticastDelegate
{

};