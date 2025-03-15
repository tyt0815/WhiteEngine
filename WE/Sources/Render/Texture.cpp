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

FTextureManager::FTextureManager():
	mTextures(ETT_None)
{
	FDXResourceManager* DRM = GetDXResourceManagerPtr();
	DRM->FlushAndExecuteCommand(&FTextureManager::LoadTextures, this);
	BuildShaderResourceDescriptorHeap();
	BuildShaderResource();
}

FTextureManager::~FTextureManager()
{

}

void FTextureManager::LoadTextures(ID3D12Device* Device, ID3D12GraphicsCommandList* CommandList)
{
	LoadTexture(ETT_Default, L"Default.dds", Device, CommandList);
	LoadTexture(ETT_White, L"White.dds", Device, CommandList);

	LoadTexture(ETT_RustedIron2_BaseColor, L"RustedIron2_BaseColor.dds", Device, CommandList);
	LoadTexture(ETT_RustedIron2_Normal, L"RustedIron2_Normal.dds", Device, CommandList);
	LoadTexture(ETT_RustedIron2_Metallic, L"RustedIron2_Metallic.dds", Device, CommandList);
	LoadTexture(ETT_RustedIron2_Roughness, L"RustedIron2_Roughness.dds", Device, CommandList);

	LoadTexture(ETT_ScuffedGold_BaseColor, L"ScuffedGold_BaseColor.dds", Device, CommandList);
	LoadTexture(ETT_ScuffedGold_Normal, L"ScuffedGold_Normal.dds", Device, CommandList);
	LoadTexture(ETT_ScuffedGold_Metallic, L"ScuffedGold_Metallic.dds", Device, CommandList);
	LoadTexture(ETT_ScuffedGold_Roughness, L"ScuffedGold_Roughness.dds", Device, CommandList);

	LoadTexture(ETT_IceField_BaseColor, L"IceField_BaseColor.dds", Device, CommandList);
	LoadTexture(ETT_IceField_Normal, L"IceField_Normal.dds", Device, CommandList);
	LoadTexture(ETT_IceField_Metallic, L"IceField_Metallic.dds", Device, CommandList);
	LoadTexture(ETT_IceField_Roughness, L"IceField_Roughness.dds", Device, CommandList);
}

void FTextureManager::LoadTexture(ETextureType Type, std::wstring Filename, ID3D12Device* Device, ID3D12GraphicsCommandList* CommandList)
{
	static std::wstring Path = L"./Resources/Textures/";
	std::unique_ptr<FTexture> Texture = std::make_unique<FTexture>();
	Texture->Type = Type;
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
	mTextures[Type] = std::move(Texture);
}

void FTextureManager::BuildShaderResourceDescriptorHeap()
{
	FDXResourceManager* DRM = GetDXResourceManagerPtr();
	ID3D12Device* Device = DRM->GetDevicePtr();
	D3D12_DESCRIPTOR_HEAP_DESC SRVHeapDesc;
	ZeroMemory(&SRVHeapDesc, sizeof(D3D12_DESCRIPTOR_HEAP_DESC));
	SRVHeapDesc.NumDescriptors = 1024;
	SRVHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	SRVHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	THROW_IF_FAILED(
		Device->CreateDescriptorHeap(
			&SRVHeapDesc,
			IID_PPV_ARGS(mSRVHeap.GetAddressOf())
		)
	);
}

void FTextureManager::BuildShaderResource()
{
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;
	for (int i = 0; i < mTextures.size(); ++i)
	{
		FTexture* Texture = mTextures[i].get();
		ID3D12Resource* TextureBuffer = Texture->Resource.Get();
		CD3DX12_CPU_DESCRIPTOR_HANDLE SRVHandle(mSRVHeap->GetCPUDescriptorHandleForHeapStart());
		SRVHandle.Offset(Texture->Type, FDXResourceManager::GetInstance()->GetCBVSRVUAVDescriptorSize());

		srvDesc.Format = TextureBuffer->GetDesc().Format;
		srvDesc.Texture2D.MipLevels = TextureBuffer->GetDesc().MipLevels;
		GetDXResourceManagerPtr()->GetDevicePtr()->CreateShaderResourceView(TextureBuffer, &srvDesc, SRVHandle);
	}
}

