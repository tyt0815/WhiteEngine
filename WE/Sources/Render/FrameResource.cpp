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

const int FrameResourcesNum = FRAME_RESOURCES_NUM;

FFrameResource::FFrameResource()
{
	ID3D12Device* Device = GetDXResourceManagerPtr()->GetDevicePtr();
	THROW_IF_FAILED(
		Device->CreateCommandAllocator(
			D3D12_COMMAND_LIST_TYPE_DIRECT,
			IID_PPV_ARGS(CommandAllocator.GetAddressOf())
		)
	);
	PassConstantBuffer = std::make_unique<TUploadBuffer<FPassConstantBuffer>>(Device, (UINT)PASS_COUNT, true);
	MeshConstantBuffer = std::make_unique<TUploadBuffer<FMeshConstantBuffer>>(Device, 1, true);
	SubmeshConstantBuffer = std::make_unique<TUploadBuffer<FSubmeshConstantBuffer>>(Device, 1, true);
	MaterialConstantBuffer = std::make_unique<TUploadBuffer<FMaterialStructuredBuffer>>(Device, EMT_None, false);
}

FFrameResourceManager::FFrameResourceManager()
{
	for (int i = 0; i < FrameResourcesNum; ++i)
	{
		mFrameResources.push_back(std::make_unique<FFrameResource>());
	}
	mTargetFrameResource = mFrameResources[mTargetFrameResourceIndex].get();

	BuildRootSignature();
}

FFrameResourceManager::~FFrameResourceManager()
{
	
}

void FFrameResourceManager::Tick()
{
    SetTargetFrameResource();
	UpdatePassCB();
	UpdateMeshCB();
	UpdateSubmeshCB();
	UpdateMaterialCB();
}

void FFrameResourceManager::FlushCommandQueues()
{
	SetTargetFrameResource();
	SetTargetFrameResource();
	SetTargetFrameResource();
}

void FFrameResourceManager::SetTargetFrameResource()
{
	FDXResourceManager* DXResourceManager = GetDXResourceManagerPtr();
	DXResourceManager->SignalFence();
	mTargetFrameResource->Fence = DXResourceManager->GetCurrentFence();
	// Cycle through the circular frame resource array.
	mTargetFrameResourceIndex = (mTargetFrameResourceIndex + 1) % FrameResourcesNum;
	mTargetFrameResource = mFrameResources[mTargetFrameResourceIndex].get();

	// Has the GPU finished processing the commands of the current frame resource?
	// If not, wait until the GPU has completed commands up to this fence point.
	ID3D12Fence* Fence = DXResourceManager->GetFencePtr();
	if (mTargetFrameResource->Fence != 0 && Fence->GetCompletedValue() < mTargetFrameResource->Fence)
	{
		HANDLE eventHandle = CreateEventEx(nullptr, nullptr, false, EVENT_ALL_ACCESS);
		THROW_IF_FAILED(Fence->SetEventOnCompletion(mTargetFrameResource->Fence, eventHandle));
		if (eventHandle)
		{
			WaitForSingleObject(eventHandle, INFINITE);
			CloseHandle(eventHandle);
		}
	}
}

void FFrameResourceManager::BuildRootSignature()
{
	CD3DX12_DESCRIPTOR_RANGE CubeTextureTable;
	CubeTextureTable.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 0);
	CD3DX12_DESCRIPTOR_RANGE TextureTable;
	TextureTable.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1024, 1, 0);

	// Tip: 자주 사용되는 것일수록 작은 인덱스에 보관하는게 퍼포먼스가 좋음
	constexpr UINT ROOT_PARAMETERs_NUM = 6;
	CD3DX12_ROOT_PARAMETER RootParameter[ROOT_PARAMETERs_NUM];
	RootParameter[0].InitAsConstantBufferView(0);	// PassCB
	RootParameter[1].InitAsConstantBufferView(1);	// MeshCB
	RootParameter[2].InitAsConstantBufferView(2);	// SubmeshCB
	RootParameter[3].InitAsShaderResourceView(0, 1);	// MaterialCB
	RootParameter[4].InitAsDescriptorTable(1, &TextureTable, D3D12_SHADER_VISIBILITY_PIXEL);	// TextureTable
	RootParameter[5].InitAsDescriptorTable(1, &CubeTextureTable, D3D12_SHADER_VISIBILITY_PIXEL);	// CubeTextureTable

	FDXUtility::BuildRootSignature(RootParameter, ROOT_PARAMETERs_NUM, mRootSignature.GetAddressOf());
}

void FFrameResourceManager::UpdatePassCB()
{
	FDXResourceManager* DeviceManager = FDXResourceManager::GetInstance();
	UTimer* Timer = GetAppTimer();

	// Build the view matrix.
	XMVECTOR target = XMVectorZero();
	XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

	WCameraComponent* Camera = GetWorld()->GetPlayerCamera();

	XMFLOAT4X4 ViewMatrix = Camera->GetViewMatrix();
	XMFLOAT4X4 ProjMatrix = Camera->GetProjMatrix();
	XMMATRIX view = XMLoadFloat4x4(&ViewMatrix);
	XMMATRIX InvView = FDXMath::GetInverseMatrix(view);
	XMMATRIX proj =  XMLoadFloat4x4(&ProjMatrix);
	XMMATRIX InvProj = FDXMath::GetInverseMatrix(proj);
	XMMATRIX ViewProj = view * proj;
	XMMATRIX InvViewProj = FDXMath::GetInverseMatrix(ViewProj);

	FPassConstantBuffer PassConstants;
	XMStoreFloat4x4(&PassConstants.View, XMMatrixTranspose(view));
	XMStoreFloat4x4(&PassConstants.InvView, XMMatrixTranspose(InvView));
	XMStoreFloat4x4(&PassConstants.Proj, XMMatrixTranspose(proj));
	XMStoreFloat4x4(&PassConstants.InvProj, XMMatrixTranspose(InvProj));
	XMStoreFloat4x4(&PassConstants.ViewProj, XMMatrixTranspose(ViewProj));
	XMStoreFloat4x4(&PassConstants.InvViewProj, XMMatrixTranspose(InvViewProj));
	PassConstants.EyePosW = Camera->GetLocation();
	D3D12_VIEWPORT Viewport = DeviceManager->GetScreenViewport();
	float Width = static_cast<float>(Viewport.Width);
	float Height = static_cast<float>(Viewport.Height);
	PassConstants.RenderTargetSize = XMFLOAT2(Width, Height);
	PassConstants.InvRenderTargetSize = XMFLOAT2(1.0f / Width, 1.0f / Height);
	PassConstants.NearZ = Camera->GetNearZ();
	PassConstants.FarZ = Camera->GetFarZ();
	PassConstants.TotalTime = Timer->GetTotalTime();
	PassConstants.DeltaTime = Timer->GetDeltaTime();

	PassConstants.AmbientLight = { 0.25f, 0.25f, 0.35f, 1.0f };

	PassConstants.FogColor = XMFLOAT4(Colors::LightSkyBlue);
	PassConstants.FogStart = 200.0f;
	PassConstants.FogRange = 100.0f;

	PassConstants.DirectionalLights[0].Direction = { 0.57735f, -0.57735f, 0.57735f };
	PassConstants.DirectionalLights[0].Color = { 1.0f, 1.0f, 1.0f };

	mTargetFrameResource->PassConstantBuffer->CopyData(0, PassConstants);
}

void FFrameResourceManager::UpdateMeshCB()
{
	bool bCBResized = false;
	if (mTargetFrameResource->MeshConstantBuffer->GetElementCount() < mMeshCBInfoPool.GetPoolSize())
	{
		mTargetFrameResource->MeshConstantBuffer = std::make_unique<TUploadBuffer<FMeshConstantBuffer>>(
			GetDXResourceManagerPtr()->GetDevicePtr(),
			(UINT)mMeshCBInfoPool.GetPoolSize(),
			true);
		bCBResized = true;
	}

	for (size_t i = 0; i < mMeshCBInfoPool.GetPoolSize(); ++i)
	{
		if (mMeshCBInfoPool.IsUsed(i) && (mMeshCBInfoPool.GetItemRef(i).DirtyFrameCount > 0 || bCBResized))
		{
			FMeshCBInfo& ObjectCBInfo = mMeshCBInfoPool.GetItemRef(i);
			mTargetFrameResource->MeshConstantBuffer->CopyData((UINT)i, ObjectCBInfo.MeshCB);
			ObjectCBInfo.DirtyFrameCount = bCBResized ? FrameResourcesNum - 1 : ObjectCBInfo.DirtyFrameCount - 1;
		}
	}
}

void FFrameResourceManager::UpdateSubmeshCB()
{
	bool bCBResized = false;
	if (mTargetFrameResource->SubmeshConstantBuffer->GetElementCount() < mSubmeshCBInfoPool.GetPoolSize())
	{
		mTargetFrameResource->SubmeshConstantBuffer = std::make_unique<TUploadBuffer<FSubmeshConstantBuffer>>(
			GetDXResourceManagerPtr()->GetDevicePtr(),
			(UINT)mSubmeshCBInfoPool.GetPoolSize(),
			true);
		bCBResized = true;
	}

	for (size_t i = 0; i < mSubmeshCBInfoPool.GetPoolSize(); ++i)
	{
		if (mSubmeshCBInfoPool.IsUsed(i) && (mSubmeshCBInfoPool.GetItemRef(i).DirtyFrameCount > 0 || bCBResized))
		{
			FSubmeshCBInfo& SubmeshCBInfo = mSubmeshCBInfoPool.GetItemRef(i);
			mTargetFrameResource->SubmeshConstantBuffer->CopyData((UINT)i, SubmeshCBInfo.SubmeshCB);
			SubmeshCBInfo.DirtyFrameCount = bCBResized ? FrameResourcesNum - 1 : SubmeshCBInfo.DirtyFrameCount - 1;
		}
	}
}

void FFrameResourceManager::UpdateMaterialCB()
{
	for (std::uint16_t i = 0; i < EMT_None; ++i)
	{
		FMaterial* Material = GetMaterialManager()->GetMaterial(i);
		if (Material->DirtyFrameCount > 0)
		{
			XMMATRIX MaterialTransform = XMLoadFloat4x4(&Material->MatTransform);
			FMaterialStructuredBuffer MaterialConstants;
			MaterialConstants.AbeldoTextureIndex = Material->AlbedoSRVHeapIndex;
			MaterialConstants.MetallicTextureIndex = Material->MetallicSRVHeapIndex;
			MaterialConstants.NormalTextureIndex = Material->NormalSRVHeapIndex;
			MaterialConstants.RoughnessTextureIndex = Material->RoughnessSRVHeapIndex;
			XMStoreFloat4x4(&MaterialConstants.MatTransform, XMMatrixTranspose(MaterialTransform));

			mTargetFrameResource->MaterialConstantBuffer->CopyData(Material->Type, MaterialConstants);
			--Material->DirtyFrameCount;
		}
	}
}
