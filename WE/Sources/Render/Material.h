#pragma once

#include "Utility/DXMath.h"
#include <string>
#include <unordered_map>
#include <memory>

extern const int NumFrameResources;

// register(b2)
struct FMaterialConstants
{
	DirectX::XMFLOAT4 DiffuseAlbedo = { 1.0f, 1.0f, 1.0f, 1.0f };
	DirectX::XMFLOAT3 FresnelR0 = { 0.01f, 0.01f, 0.01f };
	float Roughness = 0.25f;

	// Used in texture mapping.
	DirectX::XMFLOAT4X4 MatTransform = UDXMath::Identity4x4();
};

enum EMaterialType : UINT16
{
	EMT_Tile0,
	EMT_Brick0,
	EMT_Stone0,
	EMT_Skull,
	EMT_Grass,
	EMT_WireFence,
	EMT_Water,
	EMT_Foliage1,
	EMT_Default
};

class FMaterial
{
public:
	
public:
	using MaterialMap = std::unordered_map<EMaterialType, std::unique_ptr<FMaterial>>;
	static MaterialMap Materials;
	static void BuildMaterial();

	// Unique material name for lookup.
	std::string Name;

	// Index into constant buffer corresponding to this material.
	int MatCBIndex = -1;

	// Index into SRV heap for diffuse texture.
	int DiffuseSrvHeapIndex = -1;

	// Index into SRV heap for normal texture.
	int NormalSrvHeapIndex = -1;

	// Dirty flag indicating the material has changed and we need to update the constant buffer.
	// Because we have a material constant buffer for each FrameResource, we have to apply the
	// update to each FrameResource.  Thus, when we modify a material we should set 
	// NumFramesDirty = gNumFrameResources so that each frame resource gets the update.
	int NumFramesDirty = NumFrameResources;

	// Material constant buffer data used for shading.
	DirectX::XMFLOAT4 DiffuseAlbedo = { 1.0f, 1.0f, 1.0f, 1.0f };
	DirectX::XMFLOAT3 FresnelR0 = { 0.01f, 0.01f, 0.01f };
	float Roughness = .25f;
	DirectX::XMFLOAT4X4 MatTransform = UDXMath::Identity4x4();
};