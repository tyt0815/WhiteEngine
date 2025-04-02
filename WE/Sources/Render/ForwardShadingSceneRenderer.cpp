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

void FForwardShadingSceneRenderer::Initialize(
	ID3D12Device* Device,
	ID3D12CommandQueue* CommandQueue,
	ID3D12GraphicsCommandList* CommandList
)
{
	Super::Initialize(Device, CommandQueue, CommandList);
	mSkyCubeMapRenderer = std::make_unique<FCubeSkyRenderer>(std::string("Snow"));
}

void FForwardShadingSceneRenderer::Render()
{
	Super::Render();

	FFrameResource* TargetFrameResource = GetTargetFrameResource();
	ID3D12CommandAllocator* TargetCommandAllocator = TargetFrameResource->GetCommandAllocatorPtr();

	ID3D12Resource* RenderTarget = GetDXResourceManagerPtr()->GetCurrentBackBufferPtr();

	THROW_IF_FAILED(TargetCommandAllocator->Reset());
	THROW_IF_FAILED(mCommandList->Reset(TargetCommandAllocator, nullptr));

	D3D12_VIEWPORT Viewport = GetDXResourceManagerPtr()->GetScreenViewport();
	D3D12_RECT ScissorRect = GetDXResourceManagerPtr()->GetScissorRect();
	mCommandList->RSSetViewports(1, &Viewport);
	mCommandList->RSSetScissorRects(1, &ScissorRect);

	D3D12_RESOURCE_BARRIER ResourceBarrier = CD3DX12_RESOURCE_BARRIER::Transition(
		RenderTarget,
		D3D12_RESOURCE_STATE_PRESENT,
		D3D12_RESOURCE_STATE_RENDER_TARGET
	);
	mCommandList->ResourceBarrier(1, &ResourceBarrier);

	D3D12_CPU_DESCRIPTOR_HANDLE RenderTargetView = GetDXResourceManagerPtr()->GetCurrentBackBufferView();
	D3D12_CPU_DESCRIPTOR_HANDLE DepthStencilView = GetDXResourceManagerPtr()->GetDepthStencilView();
	mCommandList->ClearRenderTargetView(
		RenderTargetView,
		Colors::LightSkyBlue,
		0,
		nullptr
	);
	mCommandList->ClearDepthStencilView(DepthStencilView, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);

	mCommandList->OMSetRenderTargets(1, &RenderTargetView, true, &DepthStencilView);

	////////////////////////////////////////////////////////////////////////////////

	// Render

	// Pass Constantbuffer
	mCommandList->SetGraphicsRootSignature(mRootSignature.Get());
	FTextureManager* TexManager = GetTextureManager();
	ID3D12DescriptorHeap* descriptorHeaps[] = { TexManager->GetSRVHeapPtr() };
	mCommandList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);


	ID3D12Resource* PassConstantBuffer = TargetFrameResource->GetPassCB()->Resource();
	D3D12_GPU_VIRTUAL_ADDRESS PassConstantBufferAdress = PassConstantBuffer->GetGPUVirtualAddress();
	mCommandList->SetGraphicsRootConstantBufferView(0, PassConstantBufferAdress);
	ID3D12Resource* MaterialSB = TargetFrameResource->GetMaterialSB()->Resource();
	ID3D12Resource* DirLightSB = TargetFrameResource->GetDirectionalLightSB()->Resource();
	mCommandList->SetGraphicsRootShaderResourceView(3, MaterialSB->GetGPUVirtualAddress());
	mCommandList->SetGraphicsRootShaderResourceView(4, DirLightSB->GetGPUVirtualAddress());
	mCommandList->SetGraphicsRootDescriptorTable(5, GetTextureManager()->GetTexture2DGPUSRVForHeapStart());
	mCommandList->SetGraphicsRootDescriptorTable(6, GetTextureManager()->GetTextureCubeGPUSRVForHeapStart());
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
				mCommandList->SetPipelineState(mPipelineStates[i][j].Get());
				DrawRenderItems(TargetFrameResource, mCommandList, GetRenderItemManager()->GetRenderItems(ShadingModel, BlendMode));
			}
		}
	}
	
	mSkyCubeMapRenderer->Render(mCommandList);
	////////////////////////////////////////////////////////////////////////////////

	ResourceBarrier = CD3DX12_RESOURCE_BARRIER::Transition(
		RenderTarget,
		D3D12_RESOURCE_STATE_RENDER_TARGET,
		D3D12_RESOURCE_STATE_PRESENT
	);
	mCommandList->ResourceBarrier(1, &ResourceBarrier);

	THROW_IF_FAILED(mCommandList->Close());

	ID3D12CommandList* CommandLists[] = { mCommandList };
	mCommandQueue->ExecuteCommandLists(_countof(CommandLists), CommandLists);
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

void FForwardShadingSceneRenderer::BuildPipelineStates()
{
	ID3D12Device* Device = GetDXResourceManagerPtr()->GetDevicePtr();
	D3D12_GRAPHICS_PIPELINE_STATE_DESC ForwardLitPipelineStateDesc;
	ZeroMemory(&ForwardLitPipelineStateDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
	ForwardLitPipelineStateDesc.InputLayout = { mInputLayouts["ForwardShading"].data(), (UINT)mInputLayouts["ForwardShading"].size() };
	ForwardLitPipelineStateDesc.pRootSignature = mRootSignature.Get();
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
			IID_PPV_ARGS(mPipelineStates[ESM_DefaultLit][EBM_Opaque].GetAddressOf())
		)
	);

	{
		D3D12_GRAPHICS_PIPELINE_STATE_DESC WireFramePipelineStateDesc = ForwardLitPipelineStateDesc;
		WireFramePipelineStateDesc.RasterizerState.FillMode = D3D12_FILL_MODE_WIREFRAME;
		THROW_IF_FAILED(
			Device->CreateGraphicsPipelineState(
				&WireFramePipelineStateDesc, IID_PPV_ARGS(mWireFramePipelineState.GetAddressOf())
			)
		);
	}
}

void FForwardShadingSceneRenderer::DrawRenderItems(
	FFrameResource* FrameResource,
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