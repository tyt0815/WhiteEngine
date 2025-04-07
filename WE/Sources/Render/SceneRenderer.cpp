#include "SceneRenderer.h"
#include <DirectXColors.h>
#include "RenderItemManager.h"
#include "ShapeDrawer.h"
#include "DirectX/DXResourceManager.h"
#include "GameFramework/Object/Component/CameraComponent.h"
#include "GameFramework/Object/World/World.h"
#include "Utility/Timer.h"


const int gFrameResourcesNum = FRAME_RESOURCES_NUM;

FFrameResourceBase::FFrameResourceBase(ID3D12Device* Device)
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

FFrameResourceBase::~FFrameResourceBase()
{
}

void FFrameResourceBase::Flush()
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

void FSceneRenderer::Initialize(ID3D12Device* Device)
{
	CreateFrameResources(Device);
	BuildRootSignature();
	BuildShadersAndInputLayouts();
	BuildPipelineStates(Device);
}

void FSceneRenderer::Tick()
{
	UpdateTargetFrameResource();
	FFrameResourceBase* FrameResource = GetTargetFrameResource();
	ID3D12GraphicsCommandList* CommandList = GetDXResourceManagerPtr()->GetCommandListPtr();
	ID3D12CommandAllocator* CommandAllocator = FrameResource->GetCommandAllocatorPtr();
	THROW_IF_FAILED(CommandAllocator->Reset());
	THROW_IF_FAILED(CommandList->Reset(CommandAllocator, nullptr));

	Render(CommandList, FrameResource);

	THROW_IF_FAILED(CommandList->Close());
	ID3D12CommandList* CommandLists[] = { CommandList };
	ID3D12CommandQueue* CommandQueue = GetDXResourceManagerPtr()->GetCommandQueuePtr();
	CommandQueue->ExecuteCommandLists(_countof(CommandLists), CommandLists);
	GetDXResourceManagerPtr()->PresentAndSwapBuffer();
}

void FSceneRenderer::Destroy()
{
	// 모든 CommandAllocator를 Flush
	SwitchToNextFrameResource();
	SwitchToNextFrameResource();
	SwitchToNextFrameResource();
}

void FSceneRenderer::BuildRootSignature()
{
	D3D12_DESCRIPTOR_RANGE TextureTable = GetTextureManager()->GetTexture2DDescriptorRange();
	D3D12_DESCRIPTOR_RANGE CubeTextureTable = GetTextureManager()->GetTextureCubeDescriptorRange();

	// Tip: 자주 사용되는 것일수록 작은 인덱스에 보관하는게 퍼포먼스가 좋음
	constexpr UINT ROOT_PARAMETERs_NUM = 3;
	CD3DX12_ROOT_PARAMETER RootParameter[ROOT_PARAMETERs_NUM];
	RootParameter[0].InitAsConstants(4, 0);	// IndexCB
	RootParameter[1].InitAsDescriptorTable(1, &TextureTable, D3D12_SHADER_VISIBILITY_PIXEL);	// TextureTable
	RootParameter[2].InitAsDescriptorTable(1, &CubeTextureTable, D3D12_SHADER_VISIBILITY_PIXEL);	// CubeTextureTable

	FDXUtility::BuildRootSignature(RootParameter, ROOT_PARAMETERs_NUM, mRootSignatures["DrawRectPass"].GetAddressOf());
}

void FSceneRenderer::BuildShadersAndInputLayouts()
{
	mShaders["DrawRectPassVertexShader"] = FDXUtility::CompileShader(
		L"Shaders\\DrawRectPassVertexShader.sf",
		nullptr,
		"MainVS",
		"vs_5_1"
	);
	mShaders["DrawRectPassPixelShader"] = FDXUtility::CompileShader(
		L"Shaders\\DrawRectPassPixelShader.sf",
		nullptr,
		"MainPS",
		"ps_5_1"
	);
}

void FSceneRenderer::BuildPipelineStates(ID3D12Device* Device)
{
	D3D12_GRAPHICS_PIPELINE_STATE_DESC DrawRectPassPipelineState;
	ZeroMemory(&DrawRectPassPipelineState, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
	std::vector<D3D12_INPUT_ELEMENT_DESC> InputLayout = GetDrawingRectInputLayouts();
	DrawRectPassPipelineState.InputLayout = { InputLayout.data(), (UINT)InputLayout.size()};
	DrawRectPassPipelineState.pRootSignature = mRootSignatures["DrawRectPass"].Get();
	DrawRectPassPipelineState.VS =
	{
		reinterpret_cast<BYTE*>(mShaders["DrawRectPassVertexShader"]->GetBufferPointer()),
		mShaders["DrawRectPassVertexShader"]->GetBufferSize()
	};
	DrawRectPassPipelineState.PS =
	{
		reinterpret_cast<BYTE*>(mShaders["DrawRectPassPixelShader"]->GetBufferPointer()),
		mShaders["DrawRectPassPixelShader"]->GetBufferSize()
	};
	DrawRectPassPipelineState.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	DrawRectPassPipelineState.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	DrawRectPassPipelineState.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	DrawRectPassPipelineState.SampleMask = UINT_MAX;
	DrawRectPassPipelineState.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	DrawRectPassPipelineState.NumRenderTargets = 1;
	DrawRectPassPipelineState.RTVFormats[0] = GetDXResourceManagerPtr()->GetBackbufferFormat();
	DrawRectPassPipelineState.SampleDesc.Count = 1;
	DrawRectPassPipelineState.SampleDesc.Quality = 0;
	DrawRectPassPipelineState.DSVFormat = GetDXResourceManagerPtr()->GetDepthStencilFormat();
	THROW_IF_FAILED(
		Device->CreateGraphicsPipelineState(
			&DrawRectPassPipelineState,
			IID_PPV_ARGS(mPipelineStates["DrawRectPass"].GetAddressOf())
		)
	);
}

void FSceneRenderer::UpdateFrameBuffers(FFrameResourceBase* FrameResource)
{
	UpdatePassCB(FrameResource->GetPassCB());
	UpdateMeshCB(FrameResource->GetMeshCB());
	UpdateSubmeshCB(FrameResource->GetSubmeshCB());
	UpdateMaterialSB(FrameResource->GetMaterialSB());
	UpdateDirectionalLightSB(FrameResource->GetDirectionalLightSB());
}

void FSceneRenderer::DrawRenderItems(FFrameResourceBase* FrameResource, ID3D12GraphicsCommandList* CommandList, const TPool<FRenderItemInfo>& RenderItems)
{
	ID3D12Resource* MeshConstantBuffer = FrameResource->GetMeshCB()->Resource();
	ID3D12Resource* SubmeshConstantBuffer = FrameResource->GetSubmeshCB()->Resource();

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

void FSceneRenderer::ReadyBackBuffer(ID3D12GraphicsCommandList* CommandList)
{
	std::vector<D3D12_RESOURCE_BARRIER> ResourceBarriers;
	ID3D12Resource* BackBuffer = GetDXResourceManagerPtr()->GetCurrentBackBufferPtr();
	ID3D12Resource* DepthStencilBuffer = GetDXResourceManagerPtr()->GetDepthStencilBuffer();
	ResourceBarriers.resize(2);
	ResourceBarriers[0] = CD3DX12_RESOURCE_BARRIER::Transition(
		BackBuffer,
		D3D12_RESOURCE_STATE_PRESENT,
		D3D12_RESOURCE_STATE_RENDER_TARGET
	);
	ResourceBarriers[1] = CD3DX12_RESOURCE_BARRIER::Transition(
		DepthStencilBuffer,
		D3D12_RESOURCE_STATE_DEPTH_READ,
		D3D12_RESOURCE_STATE_DEPTH_WRITE
	);
	CommandList->ResourceBarrier((UINT)ResourceBarriers.size(), ResourceBarriers.data());

	D3D12_VIEWPORT Viewport = GetDXResourceManagerPtr()->GetScreenViewport();
	D3D12_RECT ScissorRect = GetDXResourceManagerPtr()->GetScissorRect();
	CommandList->RSSetViewports(1, &Viewport);
	CommandList->RSSetScissorRects(1, &ScissorRect);

	D3D12_CPU_DESCRIPTOR_HANDLE BackBufferRtv = GetDXResourceManagerPtr()->GetCurrentBackBufferView();
	D3D12_CPU_DESCRIPTOR_HANDLE Dsv = GetDXResourceManagerPtr()->GetDepthStencilView();
	CommandList->ClearRenderTargetView(
		BackBufferRtv,
		DirectX::Colors::Black,
		0,
		nullptr
	);
	CommandList->ClearDepthStencilView(
		Dsv,
		D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL,
		1.0f,
		0,
		0,
		nullptr
	);
}

void FSceneRenderer::FinishBackBuffer(ID3D12GraphicsCommandList* CommandList)
{
	ID3D12Resource* BackBuffer = GetDXResourceManagerPtr()->GetCurrentBackBufferPtr();
	ID3D12Resource* DepthStencilBuffer = GetDXResourceManagerPtr()->GetDepthStencilBuffer();
	std::vector<D3D12_RESOURCE_BARRIER> ResourceBarriers;
	ResourceBarriers.resize(2);
	ResourceBarriers[0] = CD3DX12_RESOURCE_BARRIER::Transition(
		BackBuffer,
		D3D12_RESOURCE_STATE_RENDER_TARGET,
		D3D12_RESOURCE_STATE_PRESENT
	);
	ResourceBarriers[1] = CD3DX12_RESOURCE_BARRIER::Transition(
		DepthStencilBuffer,
		D3D12_RESOURCE_STATE_DEPTH_WRITE,
		D3D12_RESOURCE_STATE_DEPTH_READ
	);
	CommandList->ResourceBarrier((UINT)ResourceBarriers.size(), ResourceBarriers.data());
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
	FTexture* SpecularIntegral = GetTextureManager()->GetTexture2D("IndirectSpecularIntegral");
	PassConstants.IndirectSpecularIntegralTextureIndex = SpecularIntegral ? SpecularIntegral->SRVHeapIndex : -1;

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