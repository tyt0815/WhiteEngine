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

// EObjectType 정의를 Jolt와 일치시킵니다.
enum class EObjectType : uint8_t 
{
	EOT_Static		,
	EOT_Kinematic	,
	EOT_Dynamic		,
};

class FBody final
{
public:
	FBody(const JPH::BodyCreationSettings& Settings);

	~FBody();

public:
	void AddBody(bool bActivate = true);

	void RemoveBody();

	void SetPosition(XMFLOAT3 Position);

	void SetMotiontype(EObjectType ObjectType);

	void SetActivate(bool bActivate);

	FTransform GetTransform() const;

	void SetTransform(const FTransform& Transform);

private:
	FBody() = delete;

	JPH::Body* mBody;
};

std::unique_ptr<FBody> CreateBoxBody(XMFLOAT3 Size, EObjectType ObjectType);

std::unique_ptr<FBody> CreateSphereBody(float Radius, EObjectType ObjectType);