#pragma once

#include <memory>
#include <unordered_map>
#include <wrl.h>
#include "DirectX/d3dx12.h"
#include "DirectX/Resource.h"
#include "Utility/Class.h"
#include "Utility/String.h"

class FCBVSRVUAVHeap;

class FTexture : public FResource
{
public:
	FTexture(ID3D12Device* Device);
	FTexture(std::string Name, ID3D12Device* Device, ID3D12GraphicsCommandList* CommandList);

public:
	D3D12_GPU_DESCRIPTOR_HANDLE GetSRV() const;

protected:
	FTexture();
	void CreateTexture2DSRV(const D3D12_SHADER_RESOURCE_VIEW_DESC& SRVDesc);
	void CreateTextureCubeSRV(const D3D12_SHADER_RESOURCE_VIEW_DESC& SRVDesc);

	FCBVSRVUAVHeap* mCBVSRVUAVHeap;
	std::string mName;
	int mSRVIndex = -1;

private:

public:
	inline int GetSRVHeapIndex() const
	{
		return mSRVIndex;
	}
};

class FTextureManager
{
	SINGLETON(FTextureManager);
public:
	D3D12_DESCRIPTOR_RANGE GetTexture2DDescriptorRange() const;
	D3D12_DESCRIPTOR_RANGE GetTextureCubeDescriptorRange() const;
	FTexture* GetTexture(std::string Name);

private:
	void LoadTextures(ID3D12Device* Device, ID3D12GraphicsCommandList* CommandList);
	void LoadTexture(std::string Name, ID3D12Device* Device, ID3D12GraphicsCommandList* CommandList);
	std::unordered_map<std::string, std::unique_ptr<FTexture>> mTextures;
public:
	inline bool IsExist(std::string Name) const
	{
		return mTextures.find(Name) != mTextures.end();
	}
};

inline FTextureManager* GetTextureManager()
{
	return FTextureManager::GetInstance();
}