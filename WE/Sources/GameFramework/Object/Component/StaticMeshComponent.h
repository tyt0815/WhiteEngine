#pragma once
#include "PrimitiveComponent.h"
#include <array>
#include "Render/StaticMesh.h"
#include "Render/Material.h"

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
	std::array<std::array<std::vector<size_t>, EBM_None>, ESM_None> mRenderItemInfoPoolIds;
	std::vector<size_t> mSubmeshCBInfoPoolIds;
};