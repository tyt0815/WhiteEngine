#include "FrameResource.h"
#include <DirectXColors.h>
#include "Texture.h"
#include "DirectX/DXResourceManager.h"
#include "DirectX/DXException.h"
#include "GameFramework/Object/Object.h"
#include "GameFramework/Object/World/World.h"
#include "GameFramework/Object/Component/CameraComponent.h"
#include "GameFramework/Object/Component/PrimitiveComponent.h"
#include "Utility/Timer.h"

FFrameResourceManager::FFrameResourceManager()
{
	BuildRootSignature();
}

FFrameResourceManager::~FFrameResourceManager()
{
	
}

void FFrameResourceManager::BuildRootSignature()
{
	CD3DX12_DESCRIPTOR_RANGE TextureTable;
	TextureTable.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, TEXTURE2D_NUM, 0, 0);
	CD3DX12_DESCRIPTOR_RANGE CubeTextureTable;
	CubeTextureTable.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, TEXTURECUBE_NUM, 0, 1);

	// Tip: 자주 사용되는 것일수록 작은 인덱스에 보관하는게 퍼포먼스가 좋음
	constexpr UINT ROOT_PARAMETERs_NUM = 6;
	CD3DX12_ROOT_PARAMETER RootParameter[ROOT_PARAMETERs_NUM];
	RootParameter[0].InitAsConstantBufferView(0);	// PassCB
	RootParameter[1].InitAsConstantBufferView(1);	// MeshCB
	RootParameter[2].InitAsConstantBufferView(2);	// SubmeshCB
	RootParameter[3].InitAsShaderResourceView(0, 2);	// MaterialCB
	RootParameter[4].InitAsDescriptorTable(1, &TextureTable, D3D12_SHADER_VISIBILITY_PIXEL);	// TextureTable
	RootParameter[5].InitAsDescriptorTable(1, &CubeTextureTable, D3D12_SHADER_VISIBILITY_PIXEL);	// CubeTextureTable

	FDXUtility::BuildRootSignature(RootParameter, ROOT_PARAMETERs_NUM, mRootSignature.GetAddressOf());
}
