#include "ObjectVsBroadPhaseLayerFilter.h"
#include "ObjectChannel.h"
#include "BroadPhaseLayer.h"

bool FObjectVsBroadPhaseLayerFilter::ShouldCollide(JPH::ObjectLayer inLayer1, JPH::BroadPhaseLayer inLayer2) const
{
	switch (inLayer1)
	{
	case EObjectChannel::EOC_WorldStatic:
		return inLayer2 != BroadPhaseLayers::WorldStatic; // StaticMesh 이외에만 충돌
	case EObjectChannel::EOC_WorldDynamic:
		return true; // Moving collides with everything

	case EObjectChannel::EOC_PhysicsBody:
		return true;

	case EObjectChannel::EOC_Projectile:
		return inLayer2 != BroadPhaseLayers::Projectile; // Projectile 이외에만 충돌
	default:
		JPH_ASSERT(false);
		return false;
	}
}
