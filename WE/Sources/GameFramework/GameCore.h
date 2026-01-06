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

	int RunApplication(IGameApp& App, const wchar_t* ClassName, HINSTANCE hInst, int nCmdShow);
}

#define CREATE_APPLICATION(AppClass)\
int WINAPI WinMain(HINSTANCE hInst, HINSTANCE prevInst, PSTR CmdLine, int nCmdShow)\
{\
	return GameCore::RunApplication(AppClass(), L#AppClass, hInst, nCmdShow);\
}