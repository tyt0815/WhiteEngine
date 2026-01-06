#include "SceneRenderer.h"
#include <DirectXColors.h>
#include "RenderItemManager.h"
#include "ShapeDrawer.h"
#include "DirectX/DXResourceManager.h"
#include "GameFramework/Object/Component/CameraComponent.h"
#include "GameFramework/Object/World/World.h"
#include "Utility/Timer.h"
#include "DirectX/CBVSRVUAVHeap.h"

#include "SkeletalMesh.h"

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
	mLightInfoConstantBuffer = std::make_unique<TUploadBuffer<FLightInfoConstantBuffer>>(Device, 1, true);
	mSkinnedConstantBuffer = std::make_unique<TUploadBuffer<FSkinnedConstantBuffer>>(Device, 1, true);
	mMaterialStructuredBuffer = std::make_unique<TUploadBuffer<FMaterialStructuredBuffer>>(Device, EMT_None, false);
	mDirectionalLightStructuredBuffer = std::make_unique<TUploadBuffer<FDirectionalLightStructuredBuffer>>(Device, DIR_LIGHTS_NUM, false);
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
	FDXResourceManager* DXManager = GetDXResourceManagerPtr();
	D3D12_VIEWPORT Viewport = DXManager->GetScreenViewport();
	mGaussianBlurFilter = std::make_unique<FGaussianBlurFilter>(DXManager->GetDevicePtr(), (UINT)Viewport.Width, (UINT)Viewport.Height);
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

	FResource* BackBuffer = GetDXResourceManagerPtr()->GetCurrentBackBufferPtr();
	D3D12_CPU_DESCRIPTOR_HANDLE Rtv = GetDXResourceManagerPtr()->GetCurrentBackBufferView();
	D3D12_CPU_DESCRIPTOR_HANDLE Dsv = GetDXResourceManagerPtr()->GetDepthStencilView();
	D3D12_VIEWPORT Viewport = GetDXResourceManagerPtr()->GetScreenViewport();
	D3D12_RECT ScissorRect = GetDXResourceManagerPtr()->GetScissorRect();
	Render(
		CommandList,
		FrameResource,
		BackBuffer,
		Rtv,
		Dsv,
		Viewport,
		ScissorRect
	);

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

	BuildShadowMapPassRootSignature();
}

void FSceneRenderer::BuildShadowMapPassRootSignature()
{
	D3D12_DESCRIPTOR_RANGE TextureTable = GetTextureManager()->GetTexture2DDescriptorRange();
	D3D12_DESCRIPTOR_RANGE CubeTextureTable = GetTextureManager()->GetTextureCubeDescriptorRange();

	// Tip: 자주 사용되는 것일수록 작은 인덱스에 보관하는게 퍼포먼스가 좋음
	constexpr UINT ROOT_PARAMETERs_NUM = 7;
	CD3DX12_ROOT_PARAMETER RootParameter[ROOT_PARAMETERs_NUM];
	RootParameter[0].InitAsConstants(4, 3);		// ShadowMapCB
	RootParameter[1].InitAsConstantBufferView(1);		// MeshSB
	RootParameter[2].InitAsConstantBufferView(2);		// SubmeshCB
	RootParameter[3].InitAsShaderResourceView(0, 2);		// MaterialSB
	RootParameter[4].InitAsShaderResourceView(0, 3);		// LightSB
	RootParameter[5].InitAsDescriptorTable(1, &TextureTable, D3D12_SHADER_VISIBILITY_PIXEL);	// TextureTable
	RootParameter[6].InitAsDescriptorTable(1, &CubeTextureTable, D3D12_SHADER_VISIBILITY_PIXEL);	// CubeTextureTable

	FDXUtility::BuildRootSignature(RootParameter, ROOT_PARAMETERs_NUM, mRootSignatures["ShadowMapPass"].GetAddressOf());
}

void FSceneRenderer::BuildShadersAndInputLayouts()
{
	mShaders["DrawRectPassVertexShader"] = FDXUtility::CompileShader(
		L"Shaders\\DrawRectPassVertexShader.hlsl",
		nullptr,
		"MainVS",
		"vs_5_1"
	);
	mShaders["DrawRectPassPixelShader"] = FDXUtility::CompileShader(
		L"Shaders\\DrawRectPassPixelShader.hlsl",
		nullptr,
		"MainPS",
		"ps_5_1"
	);

	mInputLayouts["MeshGeometryPass"] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 32, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
	};

	mInputLayouts["MeshGeometryPass_Skinned"] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 32, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "WEIGHTS", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 44, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "BONEINDICES", 0, DXGI_FORMAT_R32G32B32A32_UINT, 0, 56, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
	};

	BuildShadowMapShaders();
}

void FSceneRenderer::BuildShadowMapShaders()
{
	mShaders["ShadowMapPassVertexShader"] = FDXUtility::CompileShader(
		L"Shaders\\ShadowMapPassVertexShader.hlsl",
		nullptr,
		"MainVS",
		"vs_5_1"
	);\
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

	BuildShadowMapPassPipelineStates(Device);
}

void FSceneRenderer::BuildShadowMapPassPipelineStates(ID3D12Device* Device)
{
	D3D12_GRAPHICS_PIPELINE_STATE_DESC ShadowMapPassPipelineState;
	ZeroMemory(&ShadowMapPassPipelineState, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
	ShadowMapPassPipelineState.InputLayout = { mInputLayouts["MeshGeometryPass"].data(), (UINT)mInputLayouts["MeshGeometryPass"].size()};
	ShadowMapPassPipelineState.pRootSignature = mRootSignatures["ShadowMapPass"].Get();
	ShadowMapPassPipelineState.VS =
	{
		reinterpret_cast<BYTE*>(mShaders["ShadowMapPassVertexShader"]->GetBufferPointer()),
		mShaders["ShadowMapPassVertexShader"]->GetBufferSize()
	};
	ShadowMapPassPipelineState.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	ShadowMapPassPipelineState.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
	ShadowMapPassPipelineState.RasterizerState.DepthBias = 10000;
	ShadowMapPassPipelineState.RasterizerState.DepthBiasClamp = 0.0f;
	ShadowMapPassPipelineState.RasterizerState.SlopeScaledDepthBias = 1.0f;
	ShadowMapPassPipelineState.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	ShadowMapPassPipelineState.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	ShadowMapPassPipelineState.SampleMask = UINT_MAX;
	ShadowMapPassPipelineState.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	ShadowMapPassPipelineState.SampleDesc.Count = 1;
	ShadowMapPassPipelineState.SampleDesc.Quality = 0;
	ShadowMapPassPipelineState.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
	THROW_IF_FAILED(
		Device->CreateGraphicsPipelineState(
			&ShadowMapPassPipelineState,
			IID_PPV_ARGS(mPipelineStates["ShadowMapPass"].GetAddressOf())
		)
	);
}

void FSceneRenderer::UpdateFrameBuffers(FFrameResourceBase* FrameResource)
{
	UpdatePassCB(FrameResource->GetPassCB());
	UpdateMeshCB(FrameResource->GetMeshCB());
	UpdateSubmeshCB(FrameResource->GetSubmeshCB());
	UpdateLightInfoCB(FrameResource->GetLightInfoCB());
	UpdateMaterialSB(FrameResource->GetMaterialSB());
	UpdateDirectionalLightSB(FrameResource->GetDirectionalLightSB());

	// TODO: 테스트용 Skinned CB 업데이트
	auto SkinnedCB = FrameResource->GetSkinnedCB();
	FSkinnedConstantBuffer SkinnedData = {};
	SkinnedModelInstance* TestModelInstance = FSkeletalMeshManager::GetInstance()->mSkeletalMesh->SkinnedModelInst.get();
	TestModelInstance->UpdateSkinnedAnimation(0.001f);
	std::copy(
		TestModelInstance->FinalTransforms.begin(),
		TestModelInstance->FinalTransforms.end(),
		&SkinnedData.BoneTransforms[0]
	);
	SkinnedCB->CopyData(0, SkinnedData);
}

void FSceneRenderer::DrawRectPass(
	ID3D12GraphicsCommandList* CommandList,
	UINT TextureSRVIndex,
	D3D12_CPU_DESCRIPTOR_HANDLE Rtv,
	D3D12_CPU_DESCRIPTOR_HANDLE Dsv,
	const D3D12_VIEWPORT& Viewport,
	std::string PipelineStateName
)
{
	D3D12_RECT ScissorRect = FDXUtility::MakeScissorRectFromViewport(Viewport);
	ClearRenderTargetAndDepthStencil(CommandList, Rtv, Dsv, ScissorRect);

	CommandList->RSSetViewports(1, &Viewport);
	CommandList->RSSetScissorRects(1, &ScissorRect);
	CommandList->OMSetRenderTargets(1, &Rtv, false, &Dsv);

	CommandList->SetPipelineState(mPipelineStates[PipelineStateName].Get());

	CommandList->SetGraphicsRootSignature(mRootSignatures["DrawRectPass"].Get());

	FCBVSRVUAVHeap* SRVHeap = GetCBVSRVUAVHeap();
	ID3D12DescriptorHeap* DescriptorHeaps[] = { SRVHeap->Get() };
	CommandList->SetDescriptorHeaps(_countof(DescriptorHeaps), DescriptorHeaps);

	CommandList->SetGraphicsRoot32BitConstants(0, 1, &TextureSRVIndex, 0);
	CommandList->SetGraphicsRootDescriptorTable(1, SRVHeap->GetTexture2DGPUDescriptorHandleStart());
	CommandList->SetGraphicsRootDescriptorTable(2, SRVHeap->GetTextureCubeGPUDescriptorHandleStart());

	DrawRect(CommandList);
}

void FSceneRenderer::DrawShadowMap(ID3D12GraphicsCommandList* CommandList, FFrameResourceBase* FrameResource)
{
	TPool<FDirectionalLightInfo>& DirLightInfoPool = GetRenderItemManager()->mDirectionalLightInfoPool;
	int TargetIndex = min(DIR_LIGHTS_NUM, (int)DirLightInfoPool.GetPoolSize());
	int ShadingModelsNum = EShadingModel::ESM_None;
	int BlendModesNum = EBlendMode::EBM_None;
	for (int i = 0; i < TargetIndex; ++i)
	{

		if (DirLightInfoPool.IsUsed(i))
		{
			FDirectionalLightInfo DirLightInfo = DirLightInfoPool.GetItem(i);
			if (DirLightInfo.bCastShadow)
			{
				DirLightInfo.ShadowMap->TransitionResourceBarrier(CommandList, D3D12_RESOURCE_STATE_DEPTH_WRITE);
				DirLightInfo.ShadowMap->Clear(CommandList);
				D3D12_CPU_DESCRIPTOR_HANDLE Dsv = DirLightInfo.ShadowMap->GetDSV();
				CommandList->OMSetRenderTargets(0, nullptr, false, &Dsv);

				D3D12_VIEWPORT Viewport = DirLightInfo.ShadowMap->GetViewport();
				D3D12_RECT ScissorRect = DirLightInfo.ShadowMap->GetScissorRect();
				CommandList->RSSetViewports(1, &Viewport);
				CommandList->RSSetScissorRects(1, &ScissorRect);

				CommandList->SetPipelineState(mPipelineStates["ShadowMapPass"].Get());

				FCBVSRVUAVHeap* SRVHeap = GetCBVSRVUAVHeap();
				ID3D12DescriptorHeap* DescriptorHeaps[] = { SRVHeap->Get() };
				CommandList->SetDescriptorHeaps(_countof(DescriptorHeaps), DescriptorHeaps);

				CommandList->SetGraphicsRootSignature(mRootSignatures["ShadowMapPass"].Get());
				CommandList->SetGraphicsRoot32BitConstant(0, i, 0);
				CommandList->SetGraphicsRootShaderResourceView(3, FrameResource->GetMaterialSB()->Resource()->GetGPUVirtualAddress());
				CommandList->SetGraphicsRootShaderResourceView(4, FrameResource->GetDirectionalLightSB()->Resource()->GetGPUVirtualAddress());
				CommandList->SetGraphicsRootDescriptorTable(5, SRVHeap->GetTexture2DGPUDescriptorHandleStart());
				CommandList->SetGraphicsRootDescriptorTable(6, SRVHeap->GetTextureCubeGPUDescriptorHandleStart());


				for (int i = 0; i < ShadingModelsNum; ++i)
				{
					for (int j = 0; j < BlendModesNum; ++j)
					{
						TPool<FStaticMeshInfo>& StaticMeshInfoPool = GetRenderItemManager()->mStaticMeshInfoPool[i][j];
						size_t PoolSize = StaticMeshInfoPool.GetPoolSize();
						for (size_t k = 0; k < PoolSize; ++k)
						{
							if (StaticMeshInfoPool.IsUsed(k) && StaticMeshInfoPool.GetItem(k).bCastShadow)
							{
								DrawStaticMesh(
									CommandList,
									FrameResource->GetMeshCB()->Resource(),
									FrameResource->GetSubmeshCB()->Resource(),
									StaticMeshInfoPool.GetItem(k)
								);
							}
						}
					}
				}

				DirLightInfo.ShadowMap->TransitionResourceBarrier(CommandList, D3D12_RESOURCE_STATE_DEPTH_READ);
			}
		}
	}
	
}

void FSceneRenderer::ClearRenderTargetAndDepthStencil(
	ID3D12GraphicsCommandList* CommandList,
	D3D12_CPU_DESCRIPTOR_HANDLE& Rtv,
	D3D12_CPU_DESCRIPTOR_HANDLE& Dsv,
	const D3D12_RECT& ScissorRect
)
{
	CommandList->ClearRenderTargetView(
		Rtv,
		DirectX::Colors::Black,
		1,
		&ScissorRect
	);
	CommandList->ClearDepthStencilView(
		Dsv,
		D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL,
		1.0f,
		0,
		1,
		&ScissorRect
	);
}

void FSceneRenderer::DrawStaticMeshs(
	ID3D12GraphicsCommandList* CommandList,
	ID3D12Resource* MeshConstantBuffer,
	ID3D12Resource* SubmeshConstantBuffer,
	FRenderItemManager* RIM
)
{
	int ShadingModelsNum = EShadingModel::ESM_None;
	int BlendModesNum = EBlendMode::EBM_None;
	for (int i = 0; i < ShadingModelsNum; ++i)
	{
		for (int j = 0; j < BlendModesNum; ++j)
		{
			TPool<FStaticMeshInfo>& StaticMeshInfoPool = RIM->mStaticMeshInfoPool[i][j];
			size_t PoolSize = StaticMeshInfoPool.GetPoolSize();
			for (size_t k = 0; k < PoolSize; ++k)
			{
				if (StaticMeshInfoPool.IsUsed(k))
				{
					DrawStaticMesh(
						CommandList,
						MeshConstantBuffer,
						SubmeshConstantBuffer,
						StaticMeshInfoPool.GetItem(k)
					);
				}
			}
		}
	}
}

void FSceneRenderer::DrawStaticMesh(
	ID3D12GraphicsCommandList* CommandList,
	ID3D12Resource* MeshConstantBuffer,
	ID3D12Resource* SubmeshConstantBuffer,
	const FStaticMeshInfo& StaticMeshInfo
)
{
	static UINT ObjectConstantBufferByteSize = FDXUtility::CalcConstantBufferByteSize(sizeof(FMeshConstantBuffer));
	static UINT SubmeshConstantBufferByteSize = FDXUtility::CalcConstantBufferByteSize(sizeof(FSubmeshConstantBuffer));

	FMaterial* Material = StaticMeshInfo.Material;

	// MeshConstantBuffer
	auto ObjectConstantBufferAddress = MeshConstantBuffer->GetGPUVirtualAddress() + StaticMeshInfo.MeshCBIndex * ObjectConstantBufferByteSize;
	CommandList->SetGraphicsRootConstantBufferView(1, ObjectConstantBufferAddress);

	// SubmeshConstantBuffer
	auto SubmeshConstantBufferAddress = SubmeshConstantBuffer->GetGPUVirtualAddress() + StaticMeshInfo.SubmeshCBIndex * SubmeshConstantBufferByteSize;
	CommandList->SetGraphicsRootConstantBufferView(2, SubmeshConstantBufferAddress);

	D3D12_VERTEX_BUFFER_VIEW VertexBufferView = StaticMeshInfo.MeshGeometry->VertexBufferView();
	D3D12_INDEX_BUFFER_VIEW IndexBufferView = StaticMeshInfo.MeshGeometry->IndexBufferView();
	CommandList->IASetVertexBuffers(0, 1, &VertexBufferView);
	CommandList->IASetIndexBuffer(&IndexBufferView);
	CommandList->IASetPrimitiveTopology(StaticMeshInfo.MeshGeometry->PrimitiveType);

	CommandList->DrawIndexedInstanced(
		StaticMeshInfo.IndexCount,
		1,
		StaticMeshInfo.StartIndexLocation,
		StaticMeshInfo.BaseVertexLocation,
		0
	);
}

void FSceneRenderer::ReadyBackBuffer(ID3D12GraphicsCommandList* CommandList)
{
	FResource* BackBuffer = GetDXResourceManagerPtr()->GetCurrentBackBufferPtr();
	FResource* DepthStencilBuffer = GetDXResourceManagerPtr()->GetDepthStencilBuffer();
	BackBuffer->TransitionResourceBarrier(CommandList, D3D12_RESOURCE_STATE_RENDER_TARGET);
	DepthStencilBuffer->TransitionResourceBarrier(CommandList, D3D12_RESOURCE_STATE_DEPTH_WRITE);
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
	FResource* BackBuffer = GetDXResourceManagerPtr()->GetCurrentBackBufferPtr();
	FResource* DepthStencilBuffer = GetDXResourceManagerPtr()->GetDepthStencilBuffer();
	BackBuffer->TransitionResourceBarrier(CommandList, D3D12_RESOURCE_STATE_PRESENT);
	DepthStencilBuffer->TransitionResourceBarrier(CommandList, D3D12_RESOURCE_STATE_DEPTH_READ);
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
	PassConstants.EyePosW = Camera->GetLocalLocation();
	D3D12_VIEWPORT Viewport = DeviceManager->GetScreenViewport();
	float Width = static_cast<float>(Viewport.Width);
	float Height = static_cast<float>(Viewport.Height);
	PassConstants.RenderTargetSize = XMFLOAT2(Width, Height);
	PassConstants.InvRenderTargetSize = XMFLOAT2(1.0f / Width, 1.0f / Height);
	PassConstants.NearZ = Camera->GetNearZ();
	PassConstants.FarZ = Camera->GetFarZ();
	PassConstants.TotalTime = Timer->GetTotalTime();
	PassConstants.DeltaTime = Timer->GetDeltaTime();
	FTexture* SpecularIntegral = nullptr;
	PassConstants.IndirectSpecularIntegralTextureIndex = -1;

	PassConstants.FogColor = XMFLOAT4(Colors::LightSkyBlue);
	PassConstants.FogStart = 200.0f;
	PassConstants.FogRange = 100.0f;

	PassConstantBuffer->CopyData(0, PassConstants);
}

void FSceneRenderer::UpdateMeshCB(TUploadBuffer<FMeshConstantBuffer>* MeshConstantBuffer)
{
	TPool<FMeshInfo>& MeshInfoPool = GetRenderItemManager()->mMeshInfoPool;
	size_t TargetIndex = MeshInfoPool.GetPoolSize();
	for (size_t i = 0; i < TargetIndex; ++i)
	{
		if (MeshInfoPool.IsUsed(i))
		{
			FMeshInfo MeshInfo = MeshInfoPool.GetItem(i);
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
				MeshInfoPool.SetItem(i, MeshInfo);
			}
		}
	}
}

void FSceneRenderer::UpdateSubmeshCB(TUploadBuffer<FSubmeshConstantBuffer>* SubmeshConstantBuffer)
{
	TPool<FSubmeshInfo>& SubmeshInfoPool = GetRenderItemManager()->mSubmeshInfoPool;
	size_t TargetIndex = SubmeshInfoPool.GetPoolSize();
	for (size_t i = 0; i < TargetIndex; ++i)
	{
		if (SubmeshInfoPool.IsUsed(i))
		{
			FSubmeshInfo SubmeshInfo = SubmeshInfoPool.GetItem(i);
			if (SubmeshInfo.DirtyFrameCount > 0)
			{
				FSubmeshConstantBuffer SubmeshCB;
				SubmeshCB.MaterialIndex = SubmeshInfo.MaterialIndex;
				SubmeshCB.SkyIrradianceCubeMapIndex = SubmeshInfo.SkyIrradianceCubeMapIndex;
				SubmeshCB.SkySpecularCubeMapIndex = SubmeshInfo.SkySpecularCubeMapIndex;
				SubmeshConstantBuffer->CopyData((int)i, SubmeshCB);

				--SubmeshInfo.DirtyFrameCount;
				SubmeshInfoPool.SetItem(i, SubmeshInfo);
			}
		}
	}
}

void FSceneRenderer::UpdateLightInfoCB(TUploadBuffer<FLightInfoConstantBuffer>* LightInfoConstantBuffer)
{
	FLightInfoConstantBuffer LightInfoCB;
	LightInfoCB.DirectionalLightNum = min((UINT)GetRenderItemManager()->mDirectionalLightInfoPool.GetPoolSize(), DIR_LIGHTS_NUM);

	LightInfoConstantBuffer->CopyData(0, LightInfoCB);
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

void FSceneRenderer::UpdateDirectionalLightSB(TUploadBuffer<FDirectionalLightStructuredBuffer>* DirectionalLightStructuredBuffer)
{
	TPool<FDirectionalLightInfo>& DirLightInfoPool = GetRenderItemManager()->mDirectionalLightInfoPool;
	int TargetIndex = min(DIR_LIGHTS_NUM, (UINT)DirLightInfoPool.GetPoolSize());
	for (int i = 0; i < TargetIndex; ++i)
	{
		if (DirLightInfoPool.IsUsed(i))
		{
			FDirectionalLightInfo DirLightInfo = DirLightInfoPool.GetItem(i);
			if (DirLightInfo.DirtyFrameCount > 0)
			{
				// TODO: 임시코드.
				FDirectionalLightStructuredBuffer DirectionalLight;
				DirectionalLight.Direction = DirLightInfo.Direction;
				DirectionalLight.Color = DirLightInfo.Color;
				DirectionalLight.ShadowMapIndex = DirLightInfo.ShadowMap->GetSRVHeapIndex();

				// TODO: 하드코딩됨
				float SphereRadius = sqrtf(100 * 100);
				// 첫번째 광원만 그림자를 드리운다.
				XMVECTOR LightDirection = XMLoadFloat3(&DirectionalLight.Direction);
				XMVECTOR LightPosition = -2.0f * SphereRadius * LightDirection;
				XMVECTOR TargetPosition = XMVectorSet(0, 0, 0, 0);
				XMVECTOR LightUp = XMVectorSet(0, 1, 0, 0);
				XMMATRIX LightView = XMMatrixLookAtLH(LightPosition, TargetPosition, LightUp);

				// 경계구를 광원 공간으로 변환한다.
				XMFLOAT3 SphereCenterLS;
				XMStoreFloat3(&SphereCenterLS, XMVector3TransformCoord(TargetPosition, LightView));

				// 장면을 감싸는 광원 공간 직교투영 시야 입체
				float l = SphereCenterLS.x - SphereRadius;
				float b = SphereCenterLS.y - SphereRadius;
				float n = SphereCenterLS.z - SphereRadius;
				float r = SphereCenterLS.x + SphereRadius;
				float t = SphereCenterLS.y + SphereRadius;
				float f = SphereCenterLS.z + SphereRadius;

				XMMATRIX LightProj = XMMatrixOrthographicOffCenterLH(l, r, b, t, n, f);

				static XMMATRIX T(
					0.5f, 0.0f, 0.0f, 0.0f,
					0.0f, -0.5f, 0.0f, 0.0f,
					0.0f, 0.0f, 1.0f, 0.0f,
					0.5f, 0.5f, 0.0f, 1.0f
				);

				XMMATRIX S = LightView * LightProj * T;

				XMStoreFloat4x4(&DirectionalLight.LightViewProj, XMMatrixTranspose(LightView * LightProj));
				XMStoreFloat4x4(&DirectionalLight.ShadowTransform, XMMatrixTranspose(S));

				DirectionalLightStructuredBuffer->CopyData(i, DirectionalLight);

				--DirLightInfo.DirtyFrameCount;
				DirLightInfoPool.SetItem(i, DirLightInfo);
			}
		}
	}
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