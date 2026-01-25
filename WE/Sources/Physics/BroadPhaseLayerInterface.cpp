#include "BroadPhaseLayerInterface.h"
#include "BroadPhaseLayer.h"

FBroadPhaseLayerInterface::FBroadPhaseLayerInterface()
{
	// Create a mapping table from object to broad phase layer
	mObjectToBroadPhase[EObjectChannel::NON_MOVING] = BroadPhaseLayers::NON_MOVING;
	mObjectToBroadPhase[EObjectChannel::MOVING] = BroadPhaseLayers::MOVING;
}

JPH::uint FBroadPhaseLayerInterface::GetNumBroadPhaseLayers() const
{
	return BroadPhaseLayers::NUM_LAYERS;
}

JPH::BroadPhaseLayer FBroadPhaseLayerInterface::GetBroadPhaseLayer(JPH::ObjectLayer inLayer) const
{
	JPH_ASSERT(inLayer < EObjectChannel::NUM_LAYERS);
	return mObjectToBroadPhase[inLayer];
}
