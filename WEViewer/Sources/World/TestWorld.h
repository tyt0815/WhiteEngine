#pragma once

#include "GameFramework/Object/World/DefaultWorld.h"

class ADenseBoxHitReactorManager;

class WTestWorld : public WDefaultWorld
{
	typedef WDefaultWorld Super;
public:
	virtual void BeginPlay() override;

	virtual void Tick(float DeltaSecond) override;

private:
	ADenseBoxHitReactorManager* mDenseBoxHitReactorManager;

	XMFLOAT3 SpawnPlatform(
		const XMFLOAT3& StartPos,
		int TotalFloors,
		int SideCountX = 5,
		int SideCountZ = 5,
		float PlatformScale = 1.0f,
		float HorizontalSpacing = 4.0f,
		float VerticalSpacing = 0.5f,
		std::vector<int> DirectionX = { 1, 0, -1, 0 },
		std::vector<int> DirectionZ = { 0, 1, 0, -1 }
	);
};