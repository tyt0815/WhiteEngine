#pragma once

#include "GameFramework/Object/Object.h"
#include <d3d12.h>

extern const int FrameResourcesNum;

class FMeshGeometry;
class FMaterial;

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
	int DirtyFrameCount = FrameResourcesNum;
	UINT ObjectConstantBufferIndex = -1;
	UINT IndexCount = 0;
	UINT StartIndexLocation = 0;
	int BaseVertexLocation = 0;

protected:
	FTransform Transform = FTransform::Default;

public:
	inline XMMATRIX GetWorldMatrix() { return Transform.GetTransformMatrix(); }
	inline FTransform GetTransform() const { return Transform; }
	inline XMFLOAT3 GetScale() const { return Transform.Scale; }
	inline XMFLOAT3 GetRotation() const { return Transform.Rotation; }
	inline XMFLOAT3 GetTranslation() const { return Transform.Translation; }
	inline void SetTransform(FTransform NewTransform) { Transform = NewTransform; }
	inline virtual void SetTranslation(XMFLOAT3 Translation) { Transform.Translation = Translation; }
	inline void SetRotation(XMFLOAT3 Rotation) { Transform.Rotation = Rotation; }
	inline void SetScale(XMFLOAT3 Scale) { Transform.Scale = Scale; }
	virtual void RotateX(float Angle) { Transform.Rotation.x += Angle; }
	virtual void RotateY(float Angle) { Transform.Rotation.y += Angle; }
	virtual void RotateZ(float Angle) { Transform.Rotation.z += Angle; }
};