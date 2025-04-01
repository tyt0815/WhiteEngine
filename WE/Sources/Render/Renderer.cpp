#include "Renderer.h"

#include <DirectXColors.h>

#include "MeshGeometry.h"
#include "CubeSkyRenderer.h"
#include "Shader.h"
#include "Texture.h"
#include "Material.h"
#include "DirectX/DXResourceManager.h"
#include "DirectX/DXException.h"
#include "DirectX/DXUtility.h"
#include "Utility/Timer.h"
#include "GameFramework/Object/Component/CameraComponent.h"
#include "GameFramework/Object/World/World.h"
#include "RenderItemManager.h"
#include "FrameResource.h"

const int gFrameResourcesNum = FRAME_RESOURCES_NUM;

FRenderer::FRenderer():
	mSkyCubeMapRenderer(std::make_unique<FCubeSkyRenderer>(std::string("Snow")))
{
	CreateFrameResources();
	BuildRootSignature();
}

FRenderer::~FRenderer()
{
	for (int i = 0; i < gFrameResourcesNum; ++i)
	{
		SetTargetFrameResource();
	}
}

void FRenderer::Render()
{
	UpdateTargetFrameResource();
	FDXResourceManager* DeviceManager = FDXResourceManager::GetInstance();
	ID3D12Device* Device = DeviceManager->GetDevicePtr();
	ID3D12GraphicsCommandList* CommandList = DeviceManager->GetCommandListPtr();
	ID3D12CommandQueue* CommandQueue = DeviceManager->GetCommandQueuePtr();
	FFrameResource* TargetFrameResource = mTargetFrameResource;
	ID3D12CommandAllocator* TargetCommandAllocator = TargetFrameResource->CommandAllocator.Get();
	WWorld* World = GetWorld();

	ID3D12Resource* RenderTarget = DeviceManager->GetCurrentBackBufferPtr();

	THROW_IF_FAILED(TargetCommandAllocator->Reset());
	THROW_IF_FAILED(CommandList->Reset(TargetCommandAllocator, nullptr));

	D3D12_VIEWPORT Viewport = DeviceManager->GetScreenViewport();
	D3D12_RECT ScissorRect = DeviceManager->GetScissorRect();
	CommandList->RSSetViewports(1, &Viewport);
	CommandList->RSSetScissorRects(1, &ScissorRect);

	D3D12_RESOURCE_BARRIER ResourceBarrier = CD3DX12_RESOURCE_BARRIER::Transition(
		RenderTarget,
		D3D12_RESOURCE_STATE_PRESENT,
		D3D12_RESOURCE_STATE_RENDER_TARGET
	);
	CommandList->ResourceBarrier(1, &ResourceBarrier);

	D3D12_CPU_DESCRIPTOR_HANDLE RenderTargetView = DeviceManager->GetCurrentBackBufferView();
	D3D12_CPU_DESCRIPTOR_HANDLE DepthStencilView = DeviceManager->GetDepthStencilView();
	CommandList->ClearRenderTargetView(
		RenderTargetView,
		Colors::LightSkyBlue,
		0,
		nullptr
	);
	CommandList->ClearDepthStencilView(DepthStencilView, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);

	CommandList->OMSetRenderTargets(1, &RenderTargetView, true, &DepthStencilView);

	////////////////////////////////////////////////////////////////////////////////

	// Render

	// Pass Constantbuffer
	CommandList->SetGraphicsRootSignature(GetFrameResourceManager()->GetRootSignaturePtr());
	FTextureManager* TexManager = GetTextureManager();
	ID3D12DescriptorHeap* descriptorHeaps[] = { TexManager->GetSRVHeapPtr() };
	CommandList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);

	auto PassConstantBuffer = TargetFrameResource->PassConstantBuffer->Resource();
	auto PassConstantBufferAdress = PassConstantBuffer->GetGPUVirtualAddress();
	CommandList->SetGraphicsRootConstantBufferView(0, PassConstantBufferAdress);
	auto MaterialConstantBuffer = TargetFrameResource->MaterialConstantBuffer->Resource();
	CommandList->SetGraphicsRootShaderResourceView(3, MaterialConstantBuffer->GetGPUVirtualAddress());
	CommandList->SetGraphicsRootDescriptorTable(4, GetTextureManager()->GetTexture2DGPUSRVForHeapStart());
	CommandList->SetGraphicsRootDescriptorTable(5, GetTextureManager()->GetTextureCubeGPUSRVForHeapStart());
	if (bWireFrame)
	{
		// TODO
	}
	else
	{
		for (size_t i = 0; i < ESM_None; ++i)
		{
			EShadingModel ShadingModel = static_cast<EShadingModel>(i);
			for (size_t j = 0; j < EBM_None; ++j)
			{
				EBlendMode BlendMode = static_cast<EBlendMode>(j);
				CommandList->SetPipelineState(GetShaderManager()->GetPipelineStatePtr(ShadingModel, BlendMode));
				DrawRenderItems(TargetFrameResource, CommandList, GetRenderItemManager()->GetRenderItems(ShadingModel, BlendMode));
			}
		}
	}
	
	mSkyCubeMapRenderer->Render(CommandList);
	////////////////////////////////////////////////////////////////////////////////

	ResourceBarrier = CD3DX12_RESOURCE_BARRIER::Transition(
		RenderTarget,
		D3D12_RESOURCE_STATE_RENDER_TARGET,
		D3D12_RESOURCE_STATE_PRESENT
	);
	CommandList->ResourceBarrier(1, &ResourceBarrier);

	THROW_IF_FAILED(CommandList->Close());

	ID3D12CommandList* CommandLists[] = { CommandList };
	CommandQueue->ExecuteCommandLists(_countof(CommandLists), CommandLists);
}

void FRenderer::CreateFrameResources()
{
	for (int i = 0; i < mFrameResources.size(); ++i)
	{
		mFrameResources[i] = std::make_unique<FFrameResource>();
		InitializeFrameResource(mFrameResources[i].get());
	}
	mTargetFrameResource = mFrameResources[mTargetFrameResourceIndex].get();
}

void FRenderer::InitializeFrameResource(FFrameResource* FrameResource)
{
	ID3D12Device* Device = GetDXResourceManagerPtr()->GetDevicePtr();
	THROW_IF_FAILED(
		Device->CreateCommandAllocator(
			D3D12_COMMAND_LIST_TYPE_DIRECT,
			IID_PPV_ARGS(FrameResource->CommandAllocator.GetAddressOf())
		)
	);
	FrameResource->PassConstantBuffer = std::make_unique<TUploadBuffer<FPassConstantBuffer>>(Device, 1, true);
	FrameResource->MeshConstantBuffer = std::make_unique<TUploadBuffer<FMeshConstantBuffer>>(Device, MESH_CB_NUM, true);
	FrameResource->SubmeshConstantBuffer = std::make_unique<TUploadBuffer<FSubmeshConstantBuffer>>(Device, SUBMESH_CB_NUM, true);
	FrameResource->MaterialConstantBuffer = std::make_unique<TUploadBuffer<FMaterialStructuredBuffer>>(Device, EMT_None, false);
}

void FRenderer::BuildRootSignature()
{
	D3D12_DESCRIPTOR_RANGE TextureTable = GetTextureManager()->GetTexture2DDescriptorRange();
	D3D12_DESCRIPTOR_RANGE CubeTextureTable = GetTextureManager()->GetTextureCubeDescriptorRange();

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

void FRenderer::UpdateTargetFrameResource()
{
	// Set Target Frame Resource
	SetTargetFrameResource();

	UpdatePassCB(mTargetFrameResource->PassConstantBuffer.get());
	UpdateMeshCB(mTargetFrameResource->MeshConstantBuffer.get());
	UpdateSubmeshCB(mTargetFrameResource->SubmeshConstantBuffer.get());
	UpdateMaterialCB(mTargetFrameResource->MaterialConstantBuffer.get());
}

void FRenderer::SetTargetFrameResource()
{
	FDXResourceManager* DXResourceManager = GetDXResourceManagerPtr();
	DXResourceManager->SignalFence();
	mTargetFrameResource->Fence = DXResourceManager->GetCurrentFence();
	mTargetFrameResourceIndex = (mTargetFrameResourceIndex + 1) % gFrameResourcesNum;
	mTargetFrameResource = mFrameResources[mTargetFrameResourceIndex].get();

	FlushFrameResourceQueue(mTargetFrameResource);
}

void FRenderer::FlushFrameResourceQueue(FFrameResource* FrameResource)
{
	ID3D12Fence* Fence = GetDXResourceManagerPtr()->GetFencePtr();
	if (FrameResource->Fence != 0 && Fence->GetCompletedValue() < FrameResource->Fence)
	{
		HANDLE eventHandle = CreateEventEx(nullptr, nullptr, false, EVENT_ALL_ACCESS);
		THROW_IF_FAILED(Fence->SetEventOnCompletion(FrameResource->Fence, eventHandle));
		if (eventHandle)
		{
			WaitForSingleObject(eventHandle, INFINITE);
			CloseHandle(eventHandle);
		}
	}
}

void FRenderer::UpdatePassCB(TUploadBuffer<FPassConstantBuffer>* PassConstantBuffer)
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
	PassConstants.AmbientLight = { 0.25f, 0.25f, 0.35f, 1.0f };

	PassConstants.FogColor = XMFLOAT4(Colors::LightSkyBlue);
	PassConstants.FogStart = 200.0f;
	PassConstants.FogRange = 100.0f;

	PassConstants.DirectionalLights[0].Direction = { 0.57735f, -0.57735f, 0.57735f };
	PassConstants.DirectionalLights[0].Color = { 1.0f, 1.0f, 1.0f };

	PassConstantBuffer->CopyData(0, PassConstants);
}

void FRenderer::UpdateMeshCB(TUploadBuffer<FMeshConstantBuffer>* MeshConstantBuffer)
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

void FRenderer::UpdateSubmeshCB(TUploadBuffer<FSubmeshConstantBuffer>* SubmeshConstantBuffer)
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

void FRenderer::UpdateMaterialCB(TUploadBuffer<FMaterialStructuredBuffer>* MaterialStructuredBuffer)
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

void FRenderer::DrawRenderItems(FFrameResource* FrameResource, ID3D12GraphicsCommandList* CommandList, const TPool<FRenderItemInfo>& RenderItems)
{
	ID3D12Resource* MeshConstantBuffer = FrameResource->MeshConstantBuffer->Resource();
	ID3D12Resource* SubmeshConstantBuffer = FrameResource->SubmeshConstantBuffer->Resource();

	UINT ObjectConstantBufferByteSize = FDXUtility::CalcConstantBufferByteSize(sizeof(FMeshConstantBuffer));
	UINT SubmeshConstantBufferByteSize = FDXUtility::CalcConstantBufferByteSize(sizeof(FSubmeshConstantBuffer));
	UINT CBVSRVUAVDescriptorSize = FDXResourceManager::GetInstance()->GetCBVSRVUAVDescriptorSize();

	for (int i = 0; i < RenderItems.GetPoolSize(); ++i)
	{
		if (RenderItems.IsUsed(i))
		{
			const FRenderItemInfo& DrawArgs = RenderItems.GetItem(i);
			FMaterial* Material = DrawArgs.Material;

			// MeshConstantBuffer
			auto ObjectConstantBufferAddress = MeshConstantBuffer->GetGPUVirtualAddress() + DrawArgs.MeshCBIndex * ObjectConstantBufferByteSize;
			CommandList->SetGraphicsRootConstantBufferView(1, ObjectConstantBufferAddress);

			// SubmeshConstantBuffer
			auto SubmeshConstantBufferAddress = SubmeshConstantBuffer->GetGPUVirtualAddress() + DrawArgs.SubmeshCBIndex * SubmeshConstantBufferByteSize;
			CommandList->SetGraphicsRootConstantBufferView(2, SubmeshConstantBufferAddress);

			D3D12_VERTEX_BUFFER_VIEW VertexBufferView = DrawArgs.MeshGeometry->VertexBufferView();
			D3D12_INDEX_BUFFER_VIEW IndexBufferView = DrawArgs.MeshGeometry->IndexBufferView();
			CommandList->IASetVertexBuffers(0, 1, &VertexBufferView);
			CommandList->IASetIndexBuffer(&IndexBufferView);
			CommandList->IASetPrimitiveTopology(DrawArgs.MeshGeometry->PrimitiveType);

			CommandList->DrawIndexedInstanced(
				DrawArgs.IndexCount,
				1,
				DrawArgs.StartIndexLocation,
				DrawArgs.BaseVertexLocation,
				0
			);
		}
	}
}
