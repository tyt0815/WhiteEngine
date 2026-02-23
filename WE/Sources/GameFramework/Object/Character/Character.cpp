#include "Character.h"
#include "Component/CapsuleComponent.h"

ACharacter::ACharacter()
{
	mCapsuleComp = CreateComponent<WCapsuleComponent>();
	SetRootComponent(mCapsuleComp->GetWeakPtr<WSceneComponent>());


}
