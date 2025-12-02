#include "RotatingDirLight.h"

void ARotatingDirLight::Tick(float Delta)
{
	Super::Tick(Delta);

	DirectX::XMFLOAT3 Rotation = GetActorRotation();
	Rotation.z += Delta * 30;
	SetActorRotation(Rotation);
}
