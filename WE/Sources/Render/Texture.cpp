#include "Texture.h"

#include "DDSTextureLoader.h"
#include "DirectX/DXException.h"
#include "DirectX/DXResourceManager.h"
#include "DirectX/CBVSRVUAVHeap.h"

D3D12_GPU_DESCRIPTOR_HANDLE FTexture::GetSRV() const
{
	return mCBVSRVUAVHeap->GetTexture2DGPUDescriptorHandle(mSRVHeapIndex);
}

FTexture::FTexture():
	mCBVSRVUAVHeap(GetCBVSRVUAVHeap())
{
}

void FTexture::CreateTexture2DSRV(const D3D12_SHADER_RESOURCE_VIEW_DESC& SRVDesc)
{
	mSRVHeapIndex = mCBVSRVUAVHeap->CreateTexture2DSRV(mResource.Get(), SRVDesc);
}

void FTexture::CreateTextureCubeSRV(const D3D12_SHADER_RESOURCE_VIEW_DESC& SRVDesc)
{
	mSRVHeapIndex = mCBVSRVUAVHeap->CreateTextureCubeSRV(mResource.Get(), SRVDesc);
}

FTextureManager::FTextureManager()
{
	GetDXResourceManagerPtr()->ExecuteAndFlushCommand(&FTextureManager::LoadTextures, this);
}

FTextureManager::~FTextureManager()
{
	
}

D3D12_DESCRIPTOR_RANGE FTextureManager::GetTexture2DDescriptorRange() const
{
	CD3DX12_DESCRIPTOR_RANGE TextureTable;
	TextureTable.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, TEXTURE2D_VIEW_NUM, 0, 0);
	return TextureTable;
}

D3D12_DESCRIPTOR_RANGE FTextureManager::GetTextureCubeDescriptorRange() const
{
	CD3DX12_DESCRIPTOR_RANGE CubeTextureTable;
	CubeTextureTable.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, TEXTURECUBE_VIEW_NUM, 0, 1);
	return CubeTextureTable;
}

FTexture* FTextureManager::GetTexture(std::string Name)
{
	if (IsExist(Name))
	{
		return mTextures[Name].get();
	}
	return nullptr;
}

void FTextureManager::LoadTextures(ID3D12Device* Device, ID3D12GraphicsCommandList* CommandList)
{
	LoadTexture("Default", Device, CommandList);
	LoadTexture("White", Device, CommandList);
	LoadTexture("Black", Device, CommandList);

	LoadTexture("RustedIron2_Albedo", Device, CommandList);
	LoadTexture("RustedIron2_Normal", Device, CommandList);
	LoadTexture("RustedIron2_Metallic", Device, CommandList);
	LoadTexture("RustedIron2_Roughness", Device, CommandList);

	LoadTexture("ScuffedGold_Albedo", Device, CommandList);
	LoadTexture("ScuffedGold_Normal", Device, CommandList);
	LoadTexture("ScuffedGold_Metallic", Device, CommandList);
	LoadTexture("ScuffedGold_Roughness", Device, CommandList);

	LoadTexture("IceField_Albedo", Device, CommandList);
	LoadTexture("IceField_Normal", Device, CommandList);
	LoadTexture("IceField_Metallic", Device, CommandList);
	LoadTexture("IceField_Roughness", Device, CommandList);

	LoadTexture("ThickMortarStonework_Albedo", Device, CommandList);
	LoadTexture("ThickMortarStonework_Normal", Device, CommandList);
	LoadTexture("ThickMortarStonework_Metallic", Device, CommandList);
	LoadTexture("ThickMortarStonework_Roughness", Device, CommandList);

	LoadTexture("LaminateFlooringBrown_Albedo", Device, CommandList);
	LoadTexture("LaminateFlooringBrown_AO", Device, CommandList);
	LoadTexture("LaminateFlooringBrown_Height", Device, CommandList);
	LoadTexture("LaminateFlooringBrown_Metallic", Device, CommandList);
	LoadTexture("LaminateFlooringBrown_Normal", Device, CommandList);
	LoadTexture("LaminateFlooringBrown_Roughness",Device, CommandList);

	LoadTexture("SpecularIntegral", Device, CommandList);

	LoadTexture("SnowCube", Device, CommandList);
	LoadTexture("DesertCube", Device, CommandList);
}

void FTextureManager::LoadTexture(std::string Name, ID3D12Device* Device, ID3D12GraphicsCommandList* CommandList)
{
	mTextures[Name] = std::make_unique<FTexture>(Name, Device, CommandList);
}

FTexture::FTexture(std::string Name, ID3D12Device* Device, ID3D12GraphicsCommandList* CommandList) :
	mName(Name),
	mCBVSRVUAVHeap(GetCBVSRVUAVHeap())
{
	static std::wstring Path = L"./Resources/Textures/";
	std::wstring FileName = std::wstring(mName.begin(), mName.end()) + L".dds";
	THROW_IF_FAILED(
		DirectX::CreateDDSTextureFromFile12(
			Device,
			CommandList,
			(Path + FileName).c_str(),
			mResource,
			mUploadHeap
		)
	);

	
	D3D12_SHADER_RESOURCE_VIEW_DESC SRVDesc;
	ZeroMemory(&SRVDesc, sizeof(D3D12_SHADER_RESOURCE_VIEW_DESC));
	SRVDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	SRVDesc.Texture2D.MostDetailedMip = 0;
	SRVDesc.Texture2D.ResourceMinLODClamp = 0.0f;
	D3D12_RESOURCE_DESC ResourceDesc = mResource->GetDesc();
	SRVDesc.Texture2D.MipLevels = ResourceDesc.MipLevels;
	SRVDesc.Format = ResourceDesc.Format;

	switch (ResourceDesc.DepthOrArraySize)
	{
		// Texture2D
	case 1:
		SRVDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		CreateTexture2DSRV(SRVDesc);
		break;

		// TextureCube
	case 6:
		SRVDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
		CreateTextureCubeSRV(SRVDesc);
		break;

	default:
		throw L"Undefined TextureType";
	}
	
}