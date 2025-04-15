#pragma once

#include "Texture.h"
#include <vector>

class FRTVHeap;

class FRenderTarget : public FTexture
{
public:
	FRenderTarget(
		UINT Width,
		UINT Height,
		UINT MipLevels = 1,
		DXGI_FORMAT Format = DXGI_FORMAT_R8G8B8A8_UNORM
	);
	void Clear(ID3D12GraphicsCommandList* CommandList, int MipLevel = 0);
	D3D12_CPU_DESCRIPTOR_HANDLE GetRTV(int MipLevel);

private:
	void CreateRenderTargetView(int MipLevel);

	FRTVHeap* mRTVHeap;
	std::vector<int> mRTVHeapIndices;
};