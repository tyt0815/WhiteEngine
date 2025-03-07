#pragma once

#include "Object.h"
#include <d3d12.h>

extern const int NumFrameResources;

class FMeshGeometry;
class FMaterial;

enum class EActorType
{
	EAT_Opaque,
	EAT_Transparency,
	EAT_AlphaTest,
	EAT_Billboard,
	EAT_None
};

class AActor : public WObject
{
public:
	virtual void Tick(float Delta) override;
	inline void SetTextureTransform(FTransform TexTransform) { TextureTransform = TexTransform; }
	inline XMMATRIX GetTextureTransformMatrix() { return TextureTransform.GetTransformMatrix(); }

	// Render Info
	FMeshGeometry* Geometry = nullptr;
	FMaterial* Material = nullptr;
	FTransform TextureTransform = FTransform::Default;
	D3D12_PRIMITIVE_TOPOLOGY PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	int NumFramesDirty = NumFrameResources;
	UINT ObjectConstantBufferIndex = -1;
	UINT IndexCount = 0;
	UINT StartIndexLocation = 0;
	int BaseVertexLocation = 0;

	
};