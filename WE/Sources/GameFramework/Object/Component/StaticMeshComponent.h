#pragma once
#include "PrimitiveComponent.h"
#include <array>
#include "Render/StaticMesh.h"
#include "Render/Material.h"

class WStaticMeshComponent : public WPrimitiveComponent
{
public:
	WStaticMeshComponent();
	void SetStaticMesh(const FStaticMesh& StaticMesh);

private:
	FStaticMesh mStaticMesh;
	std::array<std::array<std::vector<size_t>, EBM_None>, ESM_None> mStaticMeshInfoPoolIds;
	std::vector<size_t> mSubmeshCBIndices;
	bool mbCastShadow = true;

public:
	inline void SetCastShadow(bool bCast)
	{
		mbCastShadow = bCast;
	}
};