#pragma once

#include "Texture.h"

class FCBVSRVUAVHeap;

class FUnorderedAccessTexture2D : public FTexture
{
public:
	FUnorderedAccessTexture2D(
		ID3D12Device* Device,
		UINT Width,
		UINT Height,
		DXGI_FORMAT Format = DXGI_FORMAT_R8G8B8A8_UNORM
	);

private:
	void CreateTexture2DUAV(const D3D12_UNORDERED_ACCESS_VIEW_DESC& Desc);
	int mUAVIndex;
};