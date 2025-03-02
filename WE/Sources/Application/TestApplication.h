#pragma once

#include <memory>

#include "Runtime/World/TestWorld.h"
#include "Utility/Class.h"
#include "Utility/Timer.h"
#include "Window/Window.h"

class FTestApplication
{
	SINGLETON(FTestApplication)
public:
	bool Initialize();
	int Run();

private:
	void CalculateFrameStats();
	std::unique_ptr<FWindow> mWindow;
	std::unique_ptr<UTimer> mTimer;
	std::unique_ptr<WWorld> mWorld;
	WViewCamera* Camera = nullptr;

	bool bAppPaused = false;
};