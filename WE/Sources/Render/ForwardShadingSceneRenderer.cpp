#include "ForwardShadingSceneRenderer.h"

#include <DirectXColors.h>

#include "MeshGeometry.h"
#include "CubeSkyRenderer.h"
#include "Texture.h"
#include "Material.h"
#include "DirectX/DXResourceManager.h"
#include "DirectX/DXException.h"
#include "DirectX/DXUtility.h"
#include "RenderItemManager.h"

void FForwardShadingSceneRenderer::Initialize(ID3D12Device* Device)
{
	Super::Initialize(Device);
	mSkyCubeMapRenderer = std::make_unique<FCubeSkyRenderer>(std::string("Snow"));
}

void FForwardShadingSceneRenderer::Render(const FRenderingData& RenderingData)
{
	Super::Render(RenderingData);

	ID3D12GraphicsCommandList* CommandList = RenderingData.CommandList;
	FFrameResourceBase* TargetFrameResource = GetTargetFrameResource();
	ID3D12CommandAllocator* TargetCommandAllocator = TargetFrameResource->GetCommandAllocatorPtr();

	ID3D12Resource* RenderTarget = GetDXResourceManagerPtr()->GetCurrentBackBufferPtr();

	THROW_IF_FAILED(TargetCommandAllocator->Reset());
	THROW_IF_FAILED(CommandList->Reset(TargetCommandAllocator, nullptr));

	D3D12_VIEWPORT Viewport = GetDXResourceManagerPtr()->GetScreenViewport();
	D3D12_RECT ScissorRect = GetDXResourceManagerPtr()->GetScissorRect();
	CommandList->RSSetViewports(1, &Viewport);
	CommandList->RSSetScissorRects(1, &ScissorRect);

	D3D12_RESOURCE_BARRIER ResourceBarrier = CD3DX12_RESOURCE_BARRIER::Transition(
		RenderTarget,
		D3D12_RESOURCE_STATE_PRESENT,
		D3D12_RESOURCE_STATE_RENDER_TARGET
	);
	CommandList->ResourceBarrier(1, &ResourceBarrier);

	D3D12_CPU_DESCRIPTOR_HANDLE RenderTargetView = GetDXResourceManagerPtr()->GetCurrentBackBufferView();
	D3D12_CPU_DESCRIPTOR_HANDLE DepthStencilView = GetDXResourceManagerPtr()->GetDepthStencilView();
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
	CommandList->SetGraphicsRootSignature(mRootSignatures["ForwardShading"].Get());
	FTextureManager* TexManager = GetTextureManager();
	ID3D12DescriptorHeap* descriptorHeaps[] = { TexManager->GetSRVHeapPtr() };
	CommandList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);


	ID3D12Resource* PassConstantBuffer = TargetFrameResource->GetPassCB()->Resource();
	D3D12_GPU_VIRTUAL_ADDRESS PassConstantBufferAdress = PassConstantBuffer->GetGPUVirtualAddress();
	CommandList->SetGraphicsRootConstantBufferView(0, PassConstantBufferAdress);
	ID3D12Resource* MaterialSB = TargetFrameResource->GetMaterialSB()->Resource();
	ID3D12Resource* DirLightSB = TargetFrameResource->GetDirectionalLightSB()->Resource();
	CommandList->SetGraphicsRootShaderResourceView(3, MaterialSB->GetGPUVirtualAddress());
	CommandList->SetGraphicsRootShaderResourceView(4, DirLightSB->GetGPUVirtualAddress());
	CommandList->SetGraphicsRootDescriptorTable(5, GetTextureManager()->GetTexture2DGPUSRVForHeapStart());
	CommandList->SetGraphicsRootDescriptorTable(6, GetTextureManager()->GetTextureCubeGPUSRVForHeapStart());
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
				CommandList->SetPipelineState(mPipelineStates["ForwardShading_Opaque"].Get());
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
}

void FForwardShadingSceneRenderer::BuildShadersAndInputLayouts()
{
	D3D_SHADER_MACRO Defines[] = {
		//{"FOG", "1"},
		{"FORWARDSHADING", "1"},
		{NULL, NULL}
	};

	mShaders["BasePassVertexShader"] = FDXUtility::CompileShader(
		L"Shaders\\BasePassVertexShader.sf",
		nullptr,
		"MainVS",
		"vs_5_1"
	);
	mShaders["BasePassPixelShader"] = FDXUtility::CompileShader(
		L"Shaders\\BasePassPixelShader.sf",
		Defines,
		"MainPS",
		"ps_5_1"
	);

	mInputLayouts["ForwardShading"] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
	};
}

void FForwardShadingSceneRenderer::BuildPipelineStates(ID3D12Device* Device)
{
	D3D12_GRAPHICS_PIPELINE_STATE_DESC ForwardLitPipelineStateDesc;
	ZeroMemory(&ForwardLitPipelineStateDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
	ForwardLitPipelineStateDesc.InputLayout = { mInputLayouts["ForwardShading"].data(), (UINT)mInputLayouts["ForwardShading"].size() };
	ForwardLitPipelineStateDesc.pRootSignature = mRootSignatures["ForwardShading"].Get();
	ForwardLitPipelineStateDesc.VS =
	{
		reinterpret_cast<BYTE*>(mShaders["BasePassVertexShader"]->GetBufferPointer()),
		mShaders["BasePassVertexShader"]->GetBufferSize()
	};
	ForwardLitPipelineStateDesc.PS =
	{
		reinterpret_cast<BYTE*>(mShaders["BasePassPixelShader"]->GetBufferPointer()),
		mShaders["BasePassPixelShader"]->GetBufferSize()
	};
	ForwardLitPipelineStateDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	ForwardLitPipelineStateDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	ForwardLitPipelineStateDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	ForwardLitPipelineStateDesc.SampleMask = UINT_MAX;
	ForwardLitPipelineStateDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	ForwardLitPipelineStateDesc.NumRenderTargets = 1;
	ForwardLitPipelineStateDesc.RTVFormats[0] = GetDXResourceManagerPtr()->GetBackbufferFormat();
	ForwardLitPipelineStateDesc.SampleDesc.Count = GetDXResourceManagerPtr()->IsMSAAOn() ? 4 : 1;
	ForwardLitPipelineStateDesc.SampleDesc.Quality = GetDXResourceManagerPtr()->IsMSAAOn() ? (GetDXResourceManagerPtr()->GetMSAAQuality_4x() - 1) : 0;
	ForwardLitPipelineStateDesc.DSVFormat = GetDXResourceManagerPtr()->GetDepthStencilFormat();
	THROW_IF_FAILED(
		Device->CreateGraphicsPipelineState(
			&ForwardLitPipelineStateDesc,
			IID_PPV_ARGS(mPipelineStates["ForwardShading_Opaque"].GetAddressOf())
		)
	);

	{
		D3D12_GRAPHICS_PIPELINE_STATE_DESC WireFramePipelineStateDesc = ForwardLitPipelineStateDesc;
		WireFramePipelineStateDesc.RasterizerState.FillMode = D3D12_FILL_MODE_WIREFRAME;
		THROW_IF_FAILED(
			Device->CreateGraphicsPipelineState(
				&WireFramePipelineStateDesc, IID_PPV_ARGS(mPipelineStates["WireFrame"].GetAddressOf())
			)
		);
	}
}

void FForwardShadingSceneRenderer::CreateFrameResources(ID3D12Device* Device)
{
	for (int i = 0; i < mFrameResources.size(); ++i)
	{
		mFrameResources[i] = std::make_unique<FFrameResourceBase>(Device);
	}
}

void FForwardShadingSceneRenderer::BuildRootSignature()
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

	FDXUtility::BuildRootSignature(RootParameter, ROOT_PARAMETERs_NUM, mRootSignatures["ForwardShading"].GetAddressOf());
}

void FForwardShadingSceneRenderer::DrawRenderItems(
	FFrameResourceBase* FrameResource,
	ID3D12GraphicsCommandList* CommandList,
	const TPool<FRenderItemInfo>& RenderItems
)
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