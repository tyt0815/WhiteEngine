#pragma once

#include "Object.h"

extern const int NumFrameResources;

class FMeshGeometry;
class FMaterial;

class AActor : public WObject
{
public:

	// Render Info
	FMeshGeometry* Geometry = nullptr;
	FMaterial* Material = nullptr;
	XMFLOAT4X4 TexTransform = UDXMath::Identity4x4();
	D3D12_PRIMITIVE_TOPOLOGY PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	int NumFramesDirty = NumFrameResources;
	UINT ObjectConstantBufferIndex = -1;
	UINT IndexCount = 0;
	UINT StartIndexLocation = 0;
	int BaseVertexLocation = 0;
};