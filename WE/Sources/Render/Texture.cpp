#include "Texture.h"

#include "DDSTextureLoader.h"
#include "DirectX/DXException.h"
#include "DirectX/DXResourceManager.h"

FTextureManager::FTextureManager()
{
	BuildShaderResourceDescriptorHeap();
	GetDXResourceManagerPtr()->ExecuteAndFlushCommand(&FTextureManager::BuildTextures, this);
}

FTextureManager::~FTextureManager()
{
	//for (auto& Item : mTexture2Ds)
	//{
	//	auto& Texture = Item.second;
	//	if (Texture && Texture->Resource)
	//	{
	//		Texture->Resource->Release();
	//	}
	//	Texture.reset();
	//}
	//for (auto& Item : mTextureCubes)
	//{
	//	auto& Texture = Item.second;
	//	if (Texture && Texture->Resource)
	//	{
	//		Texture->Resource->Release();
	//	}
	//	Texture.reset();
	//}
	mTexture2Ds.clear();
	mTextureCubes.clear();
}

void FTextureManager::RegisterTexture2D(std::unique_ptr<FTexture> Texture2D)
{
	if (mTexture2Ds.size() >= TEXTURE2D_NUM)
	{
		throw L"최대 텍스처수 초과";
	}
	std::string Name = Texture2D->Name;
	Texture2D->SRVHeapIndex = (UINT)mTexture2Ds.size();
	mTexture2Ds[Name] = std::move(Texture2D);
	UpdateTexture2D(Name);	
}

void FTextureManager::RegisterTextureCube(std::unique_ptr<FTexture> TextureCube)
{
	if (mTextureCubes.size() >= TEXTURECUBE_NUM)
	{
		throw L"최대 텍스처수 초과";
	}
	std::string Name = TextureCube->Name;
	TextureCube->SRVHeapIndex = (UINT)mTextureCubes.size();
	mTextureCubes[Name] = std::move(TextureCube);
	UpdateTextureCube(Name);
}

void FTextureManager::UpdateTexture2D(std::string Name)
{
	FTexture* Texture = mTexture2Ds[Name].get();
	D3D12_SHADER_RESOURCE_VIEW_DESC SRVDesc = {};
	SRVDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	SRVDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	SRVDesc.Texture2D.MostDetailedMip = 0;
	SRVDesc.Texture2D.ResourceMinLODClamp = 0.0f;
	ID3D12Resource* TextureBuffer = Texture->Resource.Get();
	SRVDesc.Texture2D.MipLevels = TextureBuffer->GetDesc().MipLevels;
	SRVDesc.Format = TextureBuffer->GetDesc().Format;

	GetDXResourceManagerPtr()->GetDevicePtr()->CreateShaderResourceView(
		TextureBuffer,
		&SRVDesc,
		GetTexture2DCPUDescriptorHandle(Texture->SRVHeapIndex)
	);
}

void FTextureManager::UpdateTextureCube(std::string Name)
{
	FTexture* Texture = mTextureCubes[Name].get();
	D3D12_SHADER_RESOURCE_VIEW_DESC SRVDesc = {};
	SRVDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	SRVDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
	SRVDesc.Texture2D.MostDetailedMip = 0;
	SRVDesc.Texture2D.ResourceMinLODClamp = 0.0f;
	ID3D12Resource* TextureBuffer = Texture->Resource.Get();
	SRVDesc.Texture2D.MipLevels = TextureBuffer->GetDesc().MipLevels;
	SRVDesc.Format = TextureBuffer->GetDesc().Format;

	GetDXResourceManagerPtr()->GetDevicePtr()->CreateShaderResourceView(
		TextureBuffer,
		&SRVDesc,
		GetTextureCubeCPUDescriptorHandle(Texture->SRVHeapIndex)
	);
}

D3D12_CPU_DESCRIPTOR_HANDLE FTextureManager::GetTexture2DCPUSRVForHeapStart() const
{
	return mSRVHeap->GetCPUDescriptorHandleForHeapStart();
}

D3D12_CPU_DESCRIPTOR_HANDLE FTextureManager::GetTextureCubeCPUSRVForHeapStart() const
{
	return CD3DX12_CPU_DESCRIPTOR_HANDLE(
		GetTexture2DCPUSRVForHeapStart(),
		TEXTURE2D_NUM,
		GetDXResourceManagerPtr()->GetCBVSRVUAVDescriptorSize()
		);
}

D3D12_GPU_DESCRIPTOR_HANDLE FTextureManager::GetTexture2DGPUSRVForHeapStart() const
{
	return mSRVHeap->GetGPUDescriptorHandleForHeapStart();
}

D3D12_GPU_DESCRIPTOR_HANDLE FTextureManager::GetTextureCubeGPUSRVForHeapStart() const
{
	return CD3DX12_GPU_DESCRIPTOR_HANDLE(
		GetTexture2DGPUSRVForHeapStart(),
		TEXTURE2D_NUM, 
		GetDXResourceManagerPtr()->GetCBVSRVUAVDescriptorSize()
	);
}

D3D12_CPU_DESCRIPTOR_HANDLE FTextureManager::GetTexture2DCPUDescriptorHandle(int i) const
{
	return CD3DX12_CPU_DESCRIPTOR_HANDLE(
		GetTexture2DCPUSRVForHeapStart(),
		i,
		GetDXResourceManagerPtr()->GetCBVSRVUAVDescriptorSize()
	);
}

D3D12_CPU_DESCRIPTOR_HANDLE FTextureManager::GetTextureCubeCPUDescriptorHandle(int i) const
{
	return CD3DX12_CPU_DESCRIPTOR_HANDLE(
		GetTextureCubeCPUSRVForHeapStart(),
		i,
		GetDXResourceManagerPtr()->GetCBVSRVUAVDescriptorSize()
	);
}

D3D12_GPU_DESCRIPTOR_HANDLE FTextureManager::GetTexture2DGPUDescriptorHandle(int i) const
{
	return CD3DX12_GPU_DESCRIPTOR_HANDLE(
		GetTexture2DGPUSRVForHeapStart(),
		i,
		GetDXResourceManagerPtr()->GetCBVSRVUAVDescriptorSize()
	);
}

D3D12_GPU_DESCRIPTOR_HANDLE FTextureManager::GetTextureCubeGPUDescriptorHandle(int i) const
{
	return CD3DX12_GPU_DESCRIPTOR_HANDLE(
		GetTextureCubeGPUSRVForHeapStart(),
		i,
		GetDXResourceManagerPtr()->GetCBVSRVUAVDescriptorSize()
	);
}

D3D12_DESCRIPTOR_RANGE FTextureManager::GetTexture2DDescriptorRange() const
{
	CD3DX12_DESCRIPTOR_RANGE TextureTable;
	TextureTable.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, TEXTURE2D_NUM, 0, 0);
	return TextureTable;
}

D3D12_DESCRIPTOR_RANGE FTextureManager::GetTextureCubeDescriptorRange() const
{
	CD3DX12_DESCRIPTOR_RANGE CubeTextureTable;
	CubeTextureTable.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, TEXTURECUBE_NUM, 0, 1);
	return CubeTextureTable;
}

void FTextureManager::BuildTextures(ID3D12Device* Device, ID3D12GraphicsCommandList* CommandList)
{
	BuildTexture("Default", L"Default.dds", Device, CommandList);
	BuildTexture("White", L"White.dds", Device, CommandList);
	BuildTexture("Black", L"Black.dds", Device, CommandList);

	BuildTexture("RustedIron2_BaseColor", L"RustedIron2_BaseColor.dds", Device, CommandList);
	BuildTexture("RustedIron2_Normal", L"RustedIron2_Normal.dds", Device, CommandList);
	BuildTexture("RustedIron2_Metallic", L"RustedIron2_Metallic.dds", Device, CommandList);
	BuildTexture("RustedIron2_Roughness", L"RustedIron2_Roughness.dds", Device, CommandList);

	BuildTexture("ScuffedGold_BaseColor", L"ScuffedGold_BaseColor.dds", Device, CommandList);
	BuildTexture("ScuffedGold_Normal", L"ScuffedGold_Normal.dds", Device, CommandList);
	BuildTexture("ScuffedGold_Metallic", L"ScuffedGold_Metallic.dds", Device, CommandList);
	BuildTexture("ScuffedGold_Roughness", L"ScuffedGold_Roughness.dds", Device, CommandList);

	BuildTexture("IceField_BaseColor", L"IceField_BaseColor.dds", Device, CommandList);
	BuildTexture("IceField_Normal", L"IceField_Normal.dds", Device, CommandList);
	BuildTexture("IceField_Metallic", L"IceField_Metallic.dds", Device, CommandList);
	BuildTexture("IceField_Roughness", L"IceField_Roughness.dds", Device, CommandList);

	BuildTexture("ThickMortarStonework_Albedo", L"ThickMortarStonework_Albedo.dds", Device, CommandList);
	BuildTexture("ThickMortarStonework_Normal", L"IceField_Normal.dds", Device, CommandList);
	BuildTexture("ThickMortarStonework_Metallic", L"IceField_Metallic.dds", Device, CommandList);
	BuildTexture("ThickMortarStonework_Roughness", L"IceField_Roughness.dds", Device, CommandList);
	
	BuildTexture("SpecularIntegral", L"SpecularIntegral.dds", Device, CommandList);

	BuildCubeTexture("Snow", L"SnowCube.dds", Device, CommandList);
	BuildCubeTexture("Desert", L"DesertCube.dds", Device, CommandList);
}

std::unique_ptr<FTexture> FTextureManager::LoadTexture(
	std::string Name,
	std::wstring Filename,
	ID3D12Device* Device,
	ID3D12GraphicsCommandList* CommandList
)
{
	static std::wstring Path = L"./Resources/Textures/";
	std::unique_ptr<FTexture> Texture = std::make_unique<FTexture>();
	Texture->Name = Name;
	THROW_IF_FAILED(
		DirectX::CreateDDSTextureFromFile12(
			Device,
			CommandList,
			(Path + Filename).c_str(),
			Texture->Resource,
			Texture->UploadHeap
		)
	);
	return std::move(Texture);
}

void FTextureManager::BuildTexture(std::string Name, std::wstring Filename, ID3D12Device* Device, ID3D12GraphicsCommandList* CommandList)
{
	RegisterTexture2D(LoadTexture(Name, Filename, Device, CommandList));
}

void FTextureManager::BuildCubeTexture(std::string Name, std::wstring Filename, ID3D12Device* Device, ID3D12GraphicsCommandList* CommandList)
{
	RegisterTextureCube(LoadTexture(Name, Filename, Device, CommandList));
}

void FTextureManager::BuildShaderResourceDescriptorHeap()
{
	FDXResourceManager* DRM = GetDXResourceManagerPtr();
	ID3D12Device* Device = DRM->GetDevicePtr();
	D3D12_DESCRIPTOR_HEAP_DESC SRVHeapDesc;
	ZeroMemory(&SRVHeapDesc, sizeof(D3D12_DESCRIPTOR_HEAP_DESC));
	SRVHeapDesc.NumDescriptors = TEXTURE2D_NUM + TEXTURECUBE_NUM;
	SRVHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	SRVHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	THROW_IF_FAILED(
		Device->CreateDescriptorHeap(
			&SRVHeapDesc,
			IID_PPV_ARGS(mSRVHeap.GetAddressOf())
		)
	);
}