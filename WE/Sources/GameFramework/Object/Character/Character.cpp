#include "Character.h"
#include "Component/CapsuleComponent.h"

ACharacter::ACharacter()
{
	mCapsuleComp = CreateComponent<WCapsuleComponent>();
	SetRootComponent(mCapsuleComp->GetWeakPtr<WSceneComponent>());
	mCapsuleComp->ActivatePhysicBody();
	mCapsuleComp->SetHalfHeight(0.5);
	mCapsuleComp->SetRadius(0.25);
	mCapsuleComp->SetObjectChannel(EObjectChannel::EOC_PhysicsBody);
	mCapsuleComp->mAllowedDOFs = EAllowedDOFs::TranslationX | EAllowedDOFs::TranslationY | EAllowedDOFs::TranslationZ;
	mCapsuleComp->mGravityFactor = 0;
}
