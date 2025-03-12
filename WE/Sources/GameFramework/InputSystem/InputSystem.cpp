#include "InputSystem.h"

FInputSystemManager::FInputSystemManager()
{
	BindKeyboardAction('i', this, &FInputSystemManager::Test);
}

FInputSystemManager::~FInputSystemManager()
{

}

void FInputSystemManager::Tick()
{
	
	ProcessKeyboardActions();
}

void FInputSystemManager::ProcessKeyboardActions()
{
	for (const std::function<void()>& ActionFunction : mKeyboardActions)
	{
		ActionFunction();
	}
}

void FInputSystemManager::Test()
{
	int a = 1;
}
