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

	if (bWireFrame)
	{
		// TODO
	}
	else
	{
		DrawActors(TargetFrameResource, CommandList);
	}

	/*{
		if (!bWireFrame)
		{
			CommandList->SetPipelineState(GetShaderManager()->GetPipelineStatePtr(ESM_DefaultLit, EBM_Opaque));
		}
		else
		{
			CommandList->SetPipelineState(GetShaderManager()->GetWireFramePipelineStatePtr());
		}
		DrawActors(World->GetActorsRef()[(int)EActorType::EAT_Opaque], TargetFrameResource, CommandList);
	}

	{
		if (!bWireFrame)
		{
			CommandList->SetPipelineState(GetShaderManager()->GetPipelineStatePtr(ESM_DefaultLit, EBM_AlphaTest));
		}
		else
		{
			CommandList->SetPipelineState(GetShaderManager()->GetWireFramePipelineStatePtr());
		}
		DrawActors(World->GetActorsRef()[(int)EActorType::EAT_AlphaTest], TargetFrameResource, CommandList);
	}

	{
		if (!bWireFrame)
		{
			CommandList->SetPipelineState(GetShaderManager()->GetBillboardPipelineStatePtr());
		}
		else
		{
			CommandList->SetPipelineState(GetShaderManager()->GetWireFramePipelineStatePtr());
		}
		DrawActors(World->GetActorsRef()[(int)EActorType::EAT_Billboard], TargetFrameResource, CommandList);
	}

	{
		if (!bWireFrame)
		{
			CommandList->SetPipelineState(GetShaderManager()->GetPipelineStatePtr(ESM_DefaultLit, EBM_Transparency));
		}
		else
		{
			CommandList->SetPipelineState(GetShaderManager()->GetWireFramePipelineStatePtr());
		}
		DrawActors(World->GetActorsRef()[(int)EActorType::EAT_Transparency], TargetFrameResource, CommandList);
	}*/

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

void FRenderer::DrawActors(FFrameResource* TargetFrameResource, ID3D12GraphicsCommandList* CommandList)
{
	ID3D12DescriptorHeap* SRVHeap = GetTextureManager()->GetSRVHeapPtr();

	ID3D12Resource* ObjectConstantBuffer = TargetFrameResource->ObjectConstantBuffer->Resource();
	ID3D12Resource* MaterialConstantBuffer = TargetFrameResource->MaterialConstantBuffer->Resource();

	UINT ObjectConstantBufferByteSize = FDXUtility::CalcConstantBufferByteSize(sizeof(FObjectConstants));
	UINT MaterialConstantBufferByteSize = FDXUtility::CalcConstantBufferByteSize(sizeof(FMaterialConstants));
	UINT CBVSRVUAVDescriptorSize = FDXResourceManager::GetInstance()->GetCBVSRVUAVDescriptorSize();

	const std::vector<AActor*> AllActors = GetWorld()->GetAllActorsRef();

	for (const AActor* Actor : AllActors)
	{
		FMaterial* Material = Actor->Material;
		// TODO: 파이프라인 상태를 매번 바꾸는 것은 비효율적.
		// SetPipelineState
		CommandList->SetPipelineState(GetShaderManager()->GetPipelineStatePtr(Material->ShadingModel, Material->BlendMode));

		// ObjectConstantBuffer
		auto ObjectConstantBufferAddress = ObjectConstantBuffer->GetGPUVirtualAddress() + Actor->ObjectConstantBufferIndex * ObjectConstantBufferByteSize;
		CommandList->SetGraphicsRootConstantBufferView(1, ObjectConstantBufferAddress);

		// MaterialConstantBuffer
		auto MaterialConstantBufferAddress = MaterialConstantBuffer->GetGPUVirtualAddress() + Material->MatCBIndex * MaterialConstantBufferByteSize;
		CommandList->SetGraphicsRootConstantBufferView(2, MaterialConstantBufferAddress);

		// Texture
		CD3DX12_GPU_DESCRIPTOR_HANDLE SRVHandle(SRVHeap->GetGPUDescriptorHandleForHeapStart());
		SRVHandle.Offset(Material->DiffuseSrvHeapIndex, CBVSRVUAVDescriptorSize);
		CommandList->SetGraphicsRootDescriptorTable(3, SRVHandle);

		D3D12_VERTEX_BUFFER_VIEW VertexBufferView = Actor->Geometry->VertexBufferView();
		D3D12_INDEX_BUFFER_VIEW IndexBufferView = Actor->Geometry->IndexBufferView();
		CommandList->IASetVertexBuffers(0, 1, &VertexBufferView);
		CommandList->IASetIndexBuffer(&IndexBufferView);
		CommandList->IASetPrimitiveTopology(Actor->PrimitiveType);

		CommandList->DrawIndexedInstanced(
			Actor->IndexCount,
			1,
			Actor->StartIndexLocation,
			Actor->BaseVertexLocation,
			0
		);
	}
}

void FRenderer::DrawActors(
	const std::vector<AActor*>& DrawTargets,
	FFrameResource* TargetFrameResource,
	ID3D12GraphicsCommandList* CommandList
)
{
	int ActorCount = (int)DrawTargets.size();
	ID3D12DescriptorHeap* SRVHeap = GetTextureManager()->GetSRVHeapPtr();

	ID3D12Resource* ObjectConstantBuffer = TargetFrameResource->ObjectConstantBuffer->Resource();
	ID3D12Resource* MaterialConstantBuffer = TargetFrameResource->MaterialConstantBuffer->Resource();

	UINT ObjectConstantBufferByteSize = FDXUtility::CalcConstantBufferByteSize(sizeof(FObjectConstants));
	UINT MaterialConstantBufferByteSize = FDXUtility::CalcConstantBufferByteSize(sizeof(FMaterialConstants));
	UINT CBVSRVUAVDescriptorSize = FDXResourceManager::GetInstance()->GetCBVSRVUAVDescriptorSize();

	for (int i = 0; i < DrawTargets.size(); ++i)
	{
		AActor* Actor = DrawTargets[i];

		// ObjectConstantBuffer
		auto ObjectConstantBufferAddress = ObjectConstantBuffer->GetGPUVirtualAddress() + Actor->ObjectConstantBufferIndex * ObjectConstantBufferByteSize;
		CommandList->SetGraphicsRootConstantBufferView(1, ObjectConstantBufferAddress);

		// MaterialConstantBuffer
		auto MaterialConstantBufferAddress = MaterialConstantBuffer->GetGPUVirtualAddress() + Actor->Material->MatCBIndex * MaterialConstantBufferByteSize;
		CommandList->SetGraphicsRootConstantBufferView(2, MaterialConstantBufferAddress);

		// Texture
		CD3DX12_GPU_DESCRIPTOR_HANDLE SRVHandle(SRVHeap->GetGPUDescriptorHandleForHeapStart());
		SRVHandle.Offset(Actor->Material->DiffuseSrvHeapIndex, CBVSRVUAVDescriptorSize);
		CommandList->SetGraphicsRootDescriptorTable(3, SRVHandle);

		D3D12_VERTEX_BUFFER_VIEW VertexBufferView = Actor->Geometry->VertexBufferView();
		D3D12_INDEX_BUFFER_VIEW IndexBufferView = Actor->Geometry->IndexBufferView();
		CommandList->IASetVertexBuffers(0, 1, &VertexBufferView);
		CommandList->IASetIndexBuffer(&IndexBufferView);
		CommandList->IASetPrimitiveTopology(Actor->PrimitiveType);

		CommandList->DrawIndexedInstanced(
			Actor->IndexCount,
			1,
			Actor->StartIndexLocation,
			Actor->BaseVertexLocation,
			0
		);
	}
}
