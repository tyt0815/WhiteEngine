//***************************************************************************************
// FCubeRenderTarget.h by Frank Luna (C) 2015 All Rights Reserved.
//***************************************************************************************

#pragma once
#include <vector>
#include "DirectX/DXUtility.h"

class FTexture;

enum class CubeMapFace : int
{
	PositiveX = 0,
	NegativeX = 1,
	PositiveY = 2,
	NegativeY = 3,
	PositiveZ = 4,
	NegativeZ = 5
};

class FCubeRenderTarget
{
public:
	FCubeRenderTarget(std::string Name, UINT width, UINT height, DXGI_FORMAT format, UINT MipLevels = 1);

	FCubeRenderTarget(const FCubeRenderTarget& rhs) = delete;
	FCubeRenderTarget& operator=(const FCubeRenderTarget& rhs) = delete;
	~FCubeRenderTarget() = default;

	ID3D12Resource* Resource();
	CD3DX12_GPU_DESCRIPTOR_HANDLE Srv();
	CD3DX12_CPU_DESCRIPTOR_HANDLE Rtv(int faceIndex, int MipLevel);

	D3D12_VIEWPORT Viewport()const;
	D3D12_RECT ScissorRect()const;

	void BuildDescriptors(
		CD3DX12_CPU_DESCRIPTOR_HANDLE hCpuSrv,
		CD3DX12_GPU_DESCRIPTOR_HANDLE hGpuSrv,
		std::vector<CD3DX12_CPU_DESCRIPTOR_HANDLE> hCpuRtv[6]
	);

	void OnResize(UINT newWidth, UINT newHeight);

	FTexture* mTexture;
private:
	void BuildDescriptors();
	void BuildResource();

private:

	ID3D12Device* mDevice = nullptr;

	D3D12_VIEWPORT mViewport;
	D3D12_RECT mScissorRect;

	UINT mWidth = 0;
	UINT mHeight = 0;
	DXGI_FORMAT mFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
	UINT mMipLevels;

	CD3DX12_CPU_DESCRIPTOR_HANDLE mhCpuSrv;
	CD3DX12_GPU_DESCRIPTOR_HANDLE mhGpuSrv;
	std::vector<CD3DX12_CPU_DESCRIPTOR_HANDLE> mhCpuRtv[6];

	Microsoft::WRL::ComPtr<ID3D12Resource> mCubeMap = nullptr;

public:
	inline UINT GetMipLevels() const
	{
		return mMipLevels;
	}
};

