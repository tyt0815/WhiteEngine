#include "ObjectLayerPairFilter.h"
#include "ObjectChannel.h"

bool FObjectLayerPairFilter::ShouldCollide(JPH::ObjectLayer inObject1, JPH::ObjectLayer inObject2) const
{
	switch (inObject1)
	{
	case EObjectChannel::NON_MOVING:
		return inObject2 == EObjectChannel::MOVING; // Non moving only collides with moving
	case EObjectChannel::MOVING:
		return true; // Moving collides with everything
	default:
		JPH_ASSERT(false);
		return false;
	}
}
