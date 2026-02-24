#pragma once

#include "Pawn/Pawn.h"

class WCapsuleComponent;
class WCharacterMovementComponent;

class ACharacter : public APawn
{
public:
	ACharacter();

private:
	WCapsuleComponent* mCapsuleComp;
	WCharacterMovementComponent* mCharacterMovementComp;

public:
	__forceinline WCapsuleComponent* GetCapsule() const
	{
		return mCapsuleComp;
	}

	__forceinline WCharacterMovementComponent* GetCharacterMovementComponent() const
	{
		return mCharacterMovementComp;
	}
};