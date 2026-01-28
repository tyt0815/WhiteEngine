#include "ObjectLayerPairFilter.h"
#include "ObjectChannel.h"

bool FObjectLayerPairFilter::ShouldCollide(JPH::ObjectLayer inObject1, JPH::ObjectLayer inObject2) const
{
	switch (inObject1)
	{
	case EObjectChannel::EOC_WorldStatic:
		return inObject2 != EObjectChannel::EOC_WorldStatic; // StaticMesh 이외에만 충돌
	case EObjectChannel::EOC_WorldDynamic:
		return true; // Moving collides with everything

	case EObjectChannel::EOC_PhysicsBody:
		return true;

	case EObjectChannel::EOC_Projectile:
		return inObject2 != EObjectChannel::EOC_Projectile; // Projectile 이외에만 충돌
	default:
		JPH_ASSERT(false);
		return false;
	}
}
