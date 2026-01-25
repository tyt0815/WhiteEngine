#include "ObjectLayerPairFilter.h"
#include "ObjectChannel.h"

bool FObjectLayerPairFilter::ShouldCollide(JPH::ObjectLayer inObject1, JPH::ObjectLayer inObject2) const
{
	switch (inObject1)
	{
	case EObjectChannel::EOC_NoneMoving:
		return inObject2 == EObjectChannel::EOC_Moving; // Non moving only collides with moving
	case EObjectChannel::EOC_Moving:
		return true; // Moving collides with everything
	default:
		JPH_ASSERT(false);
		return false;
	}
}
