#pragma once
#include <windows.h>

namespace GameCore
{
	class IGameApp
	{
	public:
		IGameApp() = default;
		virtual ~IGameApp() = default;

		virtual void Startup() = 0;

		virtual void Cleanup() = 0;

		virtual void Update(float DeltaTime) = 0;
	};
}

namespace GameCore
{
	int RunApplication(IGameApp& App, const wchar_t* ClassName, HINSTANCE hInst, int nCmdShow);
}

#define CREATE_APPLICATION(AppClass)\
	int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE prevInst, PSTR CmdLine, int nCmdShow)\
	{\
		AppClass App;\
		return GameCore::RunApplication(App, L#AppClass, hInstance, nCmdShow);\
	}