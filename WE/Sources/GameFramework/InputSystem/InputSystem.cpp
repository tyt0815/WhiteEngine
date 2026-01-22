#include "InputSystem.h"
#include "Window/Window.h"

FInputSystemManager::FInputSystemManager():
	mMouseActions(EMIT_None)
{
}

FInputSystemManager::~FInputSystemManager()
{

}

void FInputSystemManager::Tick(float Delta)
{
	if (GetMainWindowPtr()->IsCaptured())
	{
		ProcessKeyboardActions(Delta);
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

void FInputSystemManager::ProcessKeyboardActions(float Delta)
{
	for (const std::function<void(float)>& ActionFunction : mKeyboardActions)
	{
		ActionFunction(Delta);
	}
}
