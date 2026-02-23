#pragma once

class IInteractionInterface
{
public:
	virtual void Interaction() = 0;

	virtual void OnBeginInteractionFocus() = 0;

	virtual void OnEndInteractionFocus() = 0;
};