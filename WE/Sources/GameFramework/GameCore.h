#pragma once
#include <windows.h>

namespace GameCore
{
	class IGameApp
	{
	public:
		IGameApp() = default;
		virtual ~IGameApp() = default;

		virtual void StartUp() = 0;

		virtual void Update() = 0;
	};

	int RunApplication
}



#define CREATE_APPLICATION(AppClass)\
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE prevInstance, PSTR cmdLine, int showCmd)\
{\
}