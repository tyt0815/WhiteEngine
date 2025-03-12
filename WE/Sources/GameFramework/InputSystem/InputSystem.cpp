#include "InputSystem.h"
#include "Window/Window.h"

FInputSystemManager::FInputSystemManager():
	mMouseActions(EMIT_None)
{
}

FInputSystemManager::~FInputSystemManager()
{

}

void FInputSystemManager::Tick()
{
	if (GetMainWindowPtr()->IsCaptured())
	{
		ProcessKeyboardActions();
	}
}

void FInputSystemManager::ProcessMouseInput(EMouseInputType Type, int X, int Y)
{
	static FMouseInputParameter Parameter;
	for (const std::function<void(FMouseInputParameter)>& ActionFunction : mMouseActions[Type])
	{
		Parameter.X = X;
		Parameter.Y = Y;
		ActionFunction(Parameter);
	}
}

void FInputSystemManager::ProcessKeyboardActions()
{
	for (const std::function<void()>& ActionFunction : mKeyboardActions)
	{
		ActionFunction();
	}
}
