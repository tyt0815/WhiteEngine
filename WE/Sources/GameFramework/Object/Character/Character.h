#pragma once

#include "Pawn/Pawn.h"

class WCapsuleComponent;

class ACharacter : public APawn
{
public:
	ACharacter();

private:
	WCapsuleComponent* mCapsuleComp;
};