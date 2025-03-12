#pragma once
#include "PrimitiveComponent.h"
#include "Render/StaticMesh.h"

class FMeshGeometry;
class FMaterial;

extern const int FrameResourcesNum;

class WStaticMeshComponent : public WPrimitiveComponent
{
public:
	WStaticMeshComponent();
	void SetStaticMesh(const FStaticMesh& StaticMesh);

private:
	FStaticMesh mStaticMesh;
	std::vector<std::uint64_t> mDrawArgsPoolIds;
};