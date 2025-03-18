#include "Texture.h"

#include "DDSTextureLoader.h"
#include "DirectX/DXException.h"
#include "DirectX/DXResourceManager.h"

std::array<const CD3DX12_STATIC_SAMPLER_DESC, 6> FTexture::GetStaticSamplers()
{
	const CD3DX12_STATIC_SAMPLER_DESC pointWrap(
		0, // shaderRegister
		D3D12_FILTER_MIN_MAG_MIP_POINT, // filter
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressU
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressV
		D3D12_TEXTURE_ADDRESS_MODE_WRAP); // addressW

	const CD3DX12_STATIC_SAMPLER_DESC pointClamp(
		1, // shaderRegister
		D3D12_FILTER_MIN_MAG_MIP_POINT, // filter
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressU
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressV
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP); // addressW

	const CD3DX12_STATIC_SAMPLER_DESC linearWrap(
		2, // shaderRegister
		D3D12_FILTER_MIN_MAG_MIP_LINEAR, // filter
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressU
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressV
		D3D12_TEXTURE_ADDRESS_MODE_WRAP); // addressW

	const CD3DX12_STATIC_SAMPLER_DESC linearClamp(
		3, // shaderRegister
		D3D12_FILTER_MIN_MAG_MIP_LINEAR, // filter
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressU
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressV
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP); // addressW

	const CD3DX12_STATIC_SAMPLER_DESC anisotropicWrap(
		4, // shaderRegister
		D3D12_FILTER_ANISOTROPIC, // filter
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressU
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressV
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressW
		0.0f,                             // mipLODBias
		8);                               // maxAnisotropy

	const CD3DX12_STATIC_SAMPLER_DESC anisotropicClamp(
		5, // shaderRegister
		D3D12_FILTER_ANISOTROPIC, // filter
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressU
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressV
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressW
		0.0f,                              // mipLODBias
		8);                                // maxAnisotropy

	return {
		pointWrap, pointClamp,
		linearWrap, linearClamp,
		anisotropicWrap, anisotropicClamp 
	};
}

FTextureManager::FTextureManager()
{
	FDXResourceManager* DRM = GetDXResourceManagerPtr();
	DRM->FlushAndExecuteCommand(&FTextureManager::BuildTextures, this);
	
	BuildShaderResourceDescriptorHeap();
	BuildShaderResource();
}

FTextureManager::~FTextureManager()
{

}

void FTextureManager::BuildTextures(ID3D12Device* Device, ID3D12GraphicsCommandList* CommandList)
{
	BuildTexture("Default", L"Default.dds", Device, CommandList);
	BuildTexture("White", L"White.dds", Device, CommandList);

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

	BuildCubeTexture("Snow", L"SnowCube.dds", Device, CommandList);
	BuildCubeTexture("Desert", L"DesertCube.dds", Device, CommandList);
}

std::unique_ptr<FTexture> FTextureManager::LoadTexture(
	std::string Name,
	UINT SRVHeapIndex,
	std::wstring Filename,
	ID3D12Device* Device,
	ID3D12GraphicsCommandList* CommandList
)
{
	static std::wstring Path = L"./Resources/Textures/";
	std::unique_ptr<FTexture> Texture = std::make_unique<FTexture>();
	Texture->Name = Name;
	Texture->SRVHeapIndex = SRVHeapIndex;
	Texture->Filename = Filename;
	THROW_IF_FAILED(
		DirectX::CreateDDSTextureFromFile12(
			Device,
			CommandList,
			(Path + Texture->Filename).c_str(),
			Texture->Resource,
			Texture->UploadHeap
		)
	);
	return std::move(Texture);
}

void FTextureManager::BuildTexture(std::string Name, std::wstring Filename, ID3D12Device* Device, ID3D12GraphicsCommandList* CommandList)
{
	mTexture2Ds[Name] = LoadTexture(Name, (UINT)mTexture2Ds.size(), Filename, Device, CommandList);
}

void FTextureManager::BuildCubeTexture(std::string Name, std::wstring Filename, ID3D12Device* Device, ID3D12GraphicsCommandList* CommandList)
{
	mTextureCubes[Name] = LoadTexture(Name, (UINT)mTextureCubes.size(), Filename, Device, CommandList);
}

void FTextureManager::BuildShaderResourceDescriptorHeap()
{
	FDXResourceManager* DRM = GetDXResourceManagerPtr();
	ID3D12Device* Device = DRM->GetDevicePtr();
	D3D12_DESCRIPTOR_HEAP_DESC SRVHeapDesc;
	ZeroMemory(&SRVHeapDesc, sizeof(D3D12_DESCRIPTOR_HEAP_DESC));
	SRVHeapDesc.NumDescriptors = (UINT)mTexture2Ds.size() + (UINT)mTextureCubes.size();
	SRVHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	SRVHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	THROW_IF_FAILED(
		Device->CreateDescriptorHeap(
			&SRVHeapDesc,
			IID_PPV_ARGS(mTexture2DSRVHeap.GetAddressOf())
		)
	);
	THROW_IF_FAILED(
		Device->CreateDescriptorHeap(
			&SRVHeapDesc,
			IID_PPV_ARGS(mTextureCubeSRVHeap.GetAddressOf())
		)
	);
}

void FTextureManager::BuildShaderResource()
{
	D3D12_SHADER_RESOURCE_VIEW_DESC SRVDesc = {};
	SRVDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	SRVDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	SRVDesc.Texture2D.MostDetailedMip = 0;
	SRVDesc.Texture2D.ResourceMinLODClamp = 0.0f;
	for (const auto& Item : mTexture2Ds)
	{
		FTexture* Texture = Item.second.get();
		ID3D12Resource* TextureBuffer = Texture->Resource.Get();
		SRVDesc.Format = TextureBuffer->GetDesc().Format;
		SRVDesc.Texture2D.MipLevels = TextureBuffer->GetDesc().MipLevels;

		CD3DX12_CPU_DESCRIPTOR_HANDLE SRVHandle(mTexture2DSRVHeap->GetCPUDescriptorHandleForHeapStart());
		SRVHandle.Offset(Texture->SRVHeapIndex, FDXResourceManager::GetInstance()->GetCBVSRVUAVDescriptorSize());
		GetDXResourceManagerPtr()->GetDevicePtr()->CreateShaderResourceView(TextureBuffer, &SRVDesc, SRVHandle);
	}

	SRVDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
	for (const auto& Item : mTextureCubes)
	{
		FTexture* Texture = Item.second.get();
		ID3D12Resource* TextureBuffer = Texture->Resource.Get();
		SRVDesc.TextureCube.MipLevels = TextureBuffer->GetDesc().MipLevels;
		SRVDesc.Format = TextureBuffer->GetDesc().Format;

		CD3DX12_CPU_DESCRIPTOR_HANDLE SRVHandle(mTextureCubeSRVHeap->GetCPUDescriptorHandleForHeapStart());
		SRVHandle.Offset(Texture->SRVHeapIndex, FDXResourceManager::GetInstance()->GetCBVSRVUAVDescriptorSize());
		GetDXResourceManagerPtr()->GetDevicePtr()->CreateShaderResourceView(TextureBuffer, &SRVDesc, SRVHandle);
	}
}

