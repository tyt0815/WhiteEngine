#pragma once
#include "ObjectChannel.h"
#include "DirectX/DXMath.h"

#include <memory>

using namespace DirectX;

namespace JPH 
{
	class Body;
	class BodyCreationSettings;
}

namespace Physics
{
	void Startup();

	void Cleanup();

	void Tick(float DeltaTime);
}

class FBody final
{
public:
	FBody(const JPH::BodyCreationSettings& Settings);

	~FBody();

public:
	void AddBody(bool bActivate = true);

	void RemoveBody();

	void SetPosition(XMFLOAT3 Position);

	FTransform GetTransform() const;

private:
	FBody() = delete;

	JPH::Body* mBody;
};

std::unique_ptr<FBody> CreateBoxBody(XMFLOAT3 Size);

std::unique_ptr<FBody> CreateSphereBody(float Radius);