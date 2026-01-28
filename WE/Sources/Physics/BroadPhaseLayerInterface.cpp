#include "BroadPhaseLayerInterface.h"
#include "BroadPhaseLayer.h"

FBroadPhaseLayerInterface::FBroadPhaseLayerInterface()
{
	// Create a mapping table from object to broad phase layer
	for (int i = 0; i < EObjectChannel::EOC_Max; ++i)
	{
		mObjectToBroadPhase[i] = JPH::BroadPhaseLayer(i);
	}
}

JPH::uint FBroadPhaseLayerInterface::GetNumBroadPhaseLayers() const
{
	return EObjectChannel::EOC_Max;
}

JPH::BroadPhaseLayer FBroadPhaseLayerInterface::GetBroadPhaseLayer(JPH::ObjectLayer inLayer) const
{
	JPH_ASSERT(inLayer < EObjectChannel::EOC_Max);
	return mObjectToBroadPhase[inLayer];
}
