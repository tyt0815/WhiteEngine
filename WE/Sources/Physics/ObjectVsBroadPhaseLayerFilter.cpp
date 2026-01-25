#include "ObjectVsBroadPhaseLayerFilter.h"
#include "ObjectChannel.h"
#include "BroadPhaseLayer.h"

bool FObjectVsBroadPhaseLayerFilter::ShouldCollide(JPH::ObjectLayer inLayer1, JPH::BroadPhaseLayer inLayer2) const
{
	switch (inLayer1)
	{
	case EObjectChannel::NON_MOVING:
		return inLayer2 == BroadPhaseLayers::MOVING;
	case EObjectChannel::MOVING:
		return true;
	default:
		JPH_ASSERT(false);
		return false;
	}
}
