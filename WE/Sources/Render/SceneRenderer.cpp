#include "SceneRenderer.h"
#include <DirectXColors.h>
#include "RenderItemManager.h"
#include "DirectX/DXResourceManager.h"
#include "GameFramework/Object/Component/CameraComponent.h"
#include "GameFramework/Object/World/World.h"
#include "Utility/Timer.h"


const int gFrameResourcesNum = FRAME_RESOURCES_NUM;

FFrameResource::FFrameResource(ID3D12Device* Device)
{
	THROW_IF_FAILED(
		Device->CreateCommandAllocator(
			D3D12_COMMAND_LIST_TYPE_DIRECT,
			IID_PPV_ARGS(mCommandAllocator.GetAddressOf())
		)
	);
	mPassConstantBuffer = std::make_unique<TUploadBuffer<FPassConstantBuffer>>(Device, 1, true);
	mMeshConstantBuffer = std::make_unique<TUploadBuffer<FMeshConstantBuffer>>(Device, MESH_CB_NUM, true);
	mSubmeshConstantBuffer = std::make_unique<TUploadBuffer<FSubmeshConstantBuffer>>(Device, SUBMESH_CB_NUM, true);
	mMaterialStructuredBuffer = std::make_unique<TUploadBuffer<FMaterialStructuredBuffer>>(Device, EMT_None, false);
	mDirectionalLightStructuredBuffer = std::make_unique<TUploadBuffer<FDirectionalLight>>(Device, DIR_LIGHTS_NUM, false);
}

FFrameResource::~FFrameResource()
{
}

void FFrameResource::Flush()
{
	ID3D12Fence* Fence = GetDXResourceManagerPtr()->GetFencePtr();
	if (mFenceCount != 0 && Fence->GetCompletedValue() < mFenceCount)
	{
		HANDLE EventHandle = CreateEventEx(nullptr, nullptr, false, EVENT_ALL_ACCESS);
		THROW_IF_FAILED(Fence->SetEventOnCompletion(mFenceCount, EventHandle));
		if (EventHandle)
		{
			WaitForSingleObject(EventHandle, INFINITE);
			CloseHandle(EventHandle);
		}
	}
}

FSceneRenderer::FSceneRenderer()
{
	
}

void FSceneRenderer::Initialize(
	ID3D12Device* Device,
	ID3D12CommandQueue* CommandQueue,
	ID3D12GraphicsCommandList* CommandList
)
{
	mDevice = Device;
	mCommandQueue = CommandQueue;
	mCommandList = CommandList;
	CreateFrameResources();
	BuildRootSignature();
	BuildShadersAndInputLayouts();
	BuildPipelineStates();
}

void FSceneRenderer::Render()
{
	UpdateTargetFrameResource();
	
}

void FSceneRenderer::Destroy()
{
	// 모든 CommandAllocator를 Flush
	SwitchToNextFrameResource();
	SwitchToNextFrameResource();
	SwitchToNextFrameResource();
}

void FSceneRenderer::CreateFrameResources()
{
	for (int i = 0; i < mFrameResources.size(); ++i)
	{
		mFrameResources[i] = std::make_unique<FFrameResource>(mDevice);
	}
}

void FSceneRenderer::BuildRootSignature()
{
	D3D12_DESCRIPTOR_RANGE TextureTable = GetTextureManager()->GetTexture2DDescriptorRange();
	D3D12_DESCRIPTOR_RANGE CubeTextureTable = GetTextureManager()->GetTextureCubeDescriptorRange();

	// Tip: 자주 사용되는 것일수록 작은 인덱스에 보관하는게 퍼포먼스가 좋음
	constexpr UINT ROOT_PARAMETERs_NUM = 7;
	CD3DX12_ROOT_PARAMETER RootParameter[ROOT_PARAMETERs_NUM];
	RootParameter[0].InitAsConstantBufferView(0);	// PassCB
	RootParameter[1].InitAsConstantBufferView(1);	// MeshCB
	RootParameter[2].InitAsConstantBufferView(2);	// SubmeshCB
	RootParameter[3].InitAsShaderResourceView(0, 2);	// MaterialSB
	RootParameter[4].InitAsShaderResourceView(0, 3);	// DirLightSB
	RootParameter[5].InitAsDescriptorTable(1, &TextureTable, D3D12_SHADER_VISIBILITY_PIXEL);	// TextureTable
	RootParameter[6].InitAsDescriptorTable(1, &CubeTextureTable, D3D12_SHADER_VISIBILITY_PIXEL);	// CubeTextureTable

	FDXUtility::BuildRootSignature(RootParameter, ROOT_PARAMETERs_NUM, mRootSignature.GetAddressOf());
}

void FSceneRenderer::UpdateFrameBuffers(FFrameResource* FrameResource)
{
	UpdatePassCB(FrameResource->GetPassCB());
	UpdateMeshCB(FrameResource->GetMeshCB());
	UpdateSubmeshCB(FrameResource->GetSubmeshCB());
	UpdateMaterialSB(FrameResource->GetMaterialSB());
	UpdateDirectionalLightSB(FrameResource->GetDirectionalLightSB());
}

void FSceneRenderer::UpdatePassCB(TUploadBuffer<FPassConstantBuffer>* PassConstantBuffer)
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
	XMMATRIX proj = XMLoadFloat4x4(&ProjMatrix);
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
	PassConstants.IndirectSpecularIntegralTextureIndex = GetTextureManager()->GetTexture2DSRVHeapIndex("IndirectSpecularIntegral");

	PassConstants.FogColor = XMFLOAT4(Colors::LightSkyBlue);
	PassConstants.FogStart = 200.0f;
	PassConstants.FogRange = 100.0f;
	PassConstants.DirLightNum = 1;

	PassConstantBuffer->CopyData(0, PassConstants);
}

void FSceneRenderer::UpdateMeshCB(TUploadBuffer<FMeshConstantBuffer>* MeshConstantBuffer)
{
	size_t TargetIndex = GetRenderItemManager()->GetMeshInfoPoolSize();
	for (size_t i = 0; i < TargetIndex; ++i)
	{
		if (GetRenderItemManager()->IsUsedMeshInfoPool(i))
		{
			FMeshInfo MeshInfo = GetRenderItemManager()->GetMeshInfo(i);
			if (MeshInfo.DirtyFrameCount > 0)
			{
				FMeshConstantBuffer MeshCB;
				XMMATRIX World = XMLoadFloat4x4(&MeshInfo.World);
				XMVECTOR Determinant = XMMatrixDeterminant(World);
				XMMATRIX InvWorld = XMMatrixInverse(&Determinant, World);
				XMStoreFloat4x4(&MeshCB.World, XMMatrixTranspose(World));
				XMStoreFloat4x4(&MeshCB.InvTransposeWorld, InvWorld);
				MeshConstantBuffer->CopyData((int)i, MeshCB);
				--MeshInfo.DirtyFrameCount;
				GetRenderItemManager()->SetMeshInfo((int)i, MeshInfo);
			}
		}
	}
}

void FSceneRenderer::UpdateSubmeshCB(TUploadBuffer<FSubmeshConstantBuffer>* SubmeshConstantBuffer)
{
	size_t TargetIndex = GetRenderItemManager()->GetSubmeshInfoPoolSize();
	for (size_t i = 0; i < TargetIndex; ++i)
	{
		if (GetRenderItemManager()->IsUsedSubmeshInfoPool(i))
		{
			FSubmeshInfo SubmeshInfo = GetRenderItemManager()->GetSubmeshInfo(i);
			if (SubmeshInfo.DirtyFrameCount > 0)
			{
				FSubmeshConstantBuffer SubmeshCB;
				SubmeshCB.MaterialIndex = SubmeshInfo.MaterialIndex;
				SubmeshCB.SkyIrradianceCubeMapIndex = SubmeshInfo.SkyIrradianceCubeMapIndex;
				SubmeshCB.SkySpecularCubeMapIndex = SubmeshInfo.SkySpecularCubeMapIndex;
				SubmeshConstantBuffer->CopyData((int)i, SubmeshCB);

				--SubmeshInfo.DirtyFrameCount;
				GetRenderItemManager()->SetSubmeshInfo((int)i, SubmeshInfo);
			}
		}
	}
}

void FSceneRenderer::UpdateMaterialSB(TUploadBuffer<FMaterialStructuredBuffer>* MaterialStructuredBuffer)
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

			MaterialStructuredBuffer->CopyData(Material->Type, MaterialConstants);
			--Material->DirtyFrameCount;
		}
	}
}

void FSceneRenderer::UpdateDirectionalLightSB(TUploadBuffer<FDirectionalLight>* DirectionalLightStructuredBuffer)
{
	// TODO: 임시코드.
	FDirectionalLight DirectionalLight;
	DirectionalLight.Direction = { 0.57735f, -0.57735f, 0.57735f };
	DirectionalLight.Color = { 1.0f, 1.0f, 1.0f };
	DirectionalLightStructuredBuffer->CopyData(0, DirectionalLight);
}

void FSceneRenderer::UpdateTargetFrameResource()
{
	SwitchToNextFrameResource();
	UpdateFrameBuffers(GetTargetFrameResource());
}

void FSceneRenderer::SwitchToNextFrameResource()
{
	FDXResourceManager* DXResourceManager = GetDXResourceManagerPtr();
	DXResourceManager->SignalFence();
	GetTargetFrameResource()->SetFenceCount(DXResourceManager->GetCurrentFence());
	mTargetFrameResourceIndex = (mTargetFrameResourceIndex + 1) % gFrameResourcesNum;
	GetTargetFrameResource()->Flush();
}
