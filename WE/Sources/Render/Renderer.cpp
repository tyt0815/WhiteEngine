#include "Renderer.h"

#include <DirectXColors.h>

#include "MeshGeometry.h"
#include "Shader.h"
#include "Texture.h"
#include "Material.h"
#include "DirectX/DXResourceManager.h"
#include "DirectX/DXException.h"
#include "DirectX/DXUtility.h"
#include "Utility/Timer.h"
#include "GameFramework/Object/World/World.h"
#include "RenderItemManager.h"

FRenderer::FRenderer()
{
	FDXResourceManager* DeviceManager = FDXResourceManager::GetInstance();
}

void FRenderer::Render(const FRenderData& RenderData)
{
	FDXResourceManager* DeviceManager = FDXResourceManager::GetInstance();
	ID3D12Device* Device = DeviceManager->GetDevicePtr();
	ID3D12GraphicsCommandList* CommandList = DeviceManager->GetCommandListPtr();
	ID3D12CommandQueue* CommandQueue = DeviceManager->GetCommandQueuePtr();
	FFrameResource* TargetFrameResource = RenderData.FrameResource;
	ID3D12CommandAllocator* TargetCommandAllocator = RenderData.FrameResource->CommandAllocator.Get();
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
	CommandList->ClearRenderTargetView(RenderTargetView, Colors::LightSkyBlue, 0, nullptr);
	CommandList->ClearDepthStencilView(DepthStencilView, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);

	CommandList->OMSetRenderTargets(1, &RenderTargetView, true, &DepthStencilView);

	////////////////////////////////////////////////////////////////////////////////

	// Render

	// Pass Constantbuffer
	ID3D12DescriptorHeap* descriptorHeaps[] = { GetTextureManager()->GetSRVHeapPtr() };
	CommandList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);
	CommandList->SetGraphicsRootSignature(GetFrameResourceManager()->GetRootSignaturePtr());

	auto PassConstantBuffer = TargetFrameResource->PassConstantBuffer->Resource();
	auto PassConstantBufferAdress = PassConstantBuffer->GetGPUVirtualAddress();
	CommandList->SetGraphicsRootConstantBufferView(0, PassConstantBufferAdress);
	auto MaterialConstantBuffer = TargetFrameResource->MaterialConstantBuffer->Resource();
	CommandList->SetGraphicsRootShaderResourceView(3, MaterialConstantBuffer->GetGPUVirtualAddress());
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

void FRenderer::DrawRenderItems(FFrameResource* FrameResource, ID3D12GraphicsCommandList* CommandList, const TPool<FRenderItemInfo>& RenderItems)
{
	ID3D12DescriptorHeap* SRVHeap = GetTextureManager()->GetSRVHeapPtr();

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

			// Texture
			CD3DX12_GPU_DESCRIPTOR_HANDLE SRVHandle(SRVHeap->GetGPUDescriptorHandleForHeapStart());
			SRVHandle.Offset(Material->DiffuseSrvHeapIndex, CBVSRVUAVDescriptorSize);
			CommandList->SetGraphicsRootDescriptorTable(4, SRVHandle);

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