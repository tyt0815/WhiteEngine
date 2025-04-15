#pragma once

#include "Texture.h"
#include "Utility/String.h"

class FDSVHeap;

class FDepthStencil final : public FTexture
{
public:
	FDepthStencil(UINT Width, UINT Height);

public:
	void Clear(ID3D12GraphicsCommandList* CommandList);
	D3D12_CPU_DESCRIPTOR_HANDLE GetDSV() const;

private:
	FDSVHeap* mDSVHeap;
	int mDSVIndex = -1;
public:
	inline virtual DXGI_FORMAT GetFormat() const override
	{
		return DXGI_FORMAT_D24_UNORM_S8_UINT;
	}
};