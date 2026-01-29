#pragma once
#include "Actor/Actor.h"

class ATargetMarker : public AActor
{
	typedef AActor Super;
public:
	ATargetMarker();

	virtual void Tick(float DeltaSecond) override;

	void CreateTargetMarkers(int Row, int Col, float GridInterval);

private:
	TArray<TArray<XMFLOAT3>> mTargetPoints;
};