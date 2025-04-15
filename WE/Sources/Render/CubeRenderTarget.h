#pragma once
#include "Texture.h"
#include <array>
#include <DirectXMath.h>

class FRTVHeap;

class FCubeRenderTarget : public FTexture
{
public:
	static std::array<DirectX::XMFLOAT4X4, 6> GetCubeMapViews();
	FCubeRenderTarget(
		UINT Width,
		UINT Height,
		UINT MipLevels = 1,
		DXGI_FORMAT Format = DXGI_FORMAT_R8G8B8A8_UNORM
	);
	void Clear(ID3D12GraphicsCommandList* CommandList, int FaceIndex, int MipLevel = 0);
	D3D12_CPU_DESCRIPTOR_HANDLE GetRTV(int FaceIndex, int MipLevel);

private:
	FRTVHeap* mRTVHeap;
	std::vector<std::vector<int>> mRTVHeapIndices;
};

