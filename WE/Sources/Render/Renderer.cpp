#include "Renderer.h"

#include <DirectXColors.h>

#include "MeshGeometry.h"
#include "Texture.h"
#include "Material.h"
#include "DirectX/DXDeviceManager.h"
#include "DirectX/DXException.h"
#include "DirectX/DXUtility.h"
#include "Utility/Timer.h"
#include "GameFramework/Object/World/World.h"

FRenderer::FRenderer()
{
	FDXResourceManager* DeviceManager = FDXResourceManager::GetInstance();
	Device = DeviceManager->GetDevicePtr();
	CommandQueue = DeviceManager->GetCommandQueuePtr();
	CommandAllocator = DeviceManager->GetCommandAllocatorPtr();
	CommandList = DeviceManager->GetCommandListPtr();
	Fence = DeviceManager->GetFencePtr();
}

bool FRenderer::Initialize()
{
	BuildDescriptorHeaps();
	BuildShaderResources();
	BuildRootSignature();
	BuildShaderAndInputLayout();
	BuildPipelineStateObject();

	return true;
}

void FRenderer::Render(const FRenderData& RenderData)
{
	FFrameResource* TargetFrameResource = RenderData.FrameResource;
	ID3D12CommandAllocator* TargetCommandAllocator = RenderData.FrameResource->CommandAllocator.Get();
	WWorld* World = GetWorld();

	FDXResourceManager* DeviceManager = FDXResourceManager::GetInstance();
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
	ID3D12DescriptorHeap* descriptorHeaps[] = { SRVHeap.Get() };
	CommandList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);
	CommandList->SetGraphicsRootSignature(RootSignature.Get());

	auto PassConstantBuffer = TargetFrameResource->PassConstantBuffer->Resource();
	auto PassConstantBufferAdress = PassConstantBuffer->GetGPUVirtualAddress();
	CommandList->SetGraphicsRootConstantBufferView(0, PassConstantBufferAdress);

	{
		if (!bWireFrame)
		{
			CommandList->SetPipelineState(PipelineStateObjects[(int)EPipelineState::EPS_Opaque].Get());
		}
		else
		{
			CommandList->SetPipelineState(PipelineStateObjects[(int)EPipelineState::EPS_WireFrame].Get());
		}
		DrawActors(World->GetActorsRef()[(int)EActorType::EAT_Opaque], TargetFrameResource);
	}

	{
		if (!bWireFrame)
		{
			CommandList->SetPipelineState(PipelineStateObjects[(int)EPipelineState::EPS_AlphaTest].Get());
		}
		else
		{
			CommandList->SetPipelineState(PipelineStateObjects[(int)EPipelineState::EPS_WireFrame].Get());
		}
		DrawActors(World->GetActorsRef()[(int)EActorType::EAT_AlphaTest], TargetFrameResource);
	}

	{
		if (!bWireFrame)
		{
			CommandList->SetPipelineState(PipelineStateObjects[(int)EPipelineState::EPS_Billboard].Get());
		}
		else
		{
			CommandList->SetPipelineState(PipelineStateObjects[(int)EPipelineState::EPS_WireFrame].Get());
		}
		DrawActors(World->GetActorsRef()[(int)EActorType::EAT_Billboard], TargetFrameResource);
	}

	{
		if (!bWireFrame)
		{
			CommandList->SetPipelineState(PipelineStateObjects[(int)EPipelineState::EPS_Transparency].Get());
		}
		else
		{
			CommandList->SetPipelineState(PipelineStateObjects[(int)EPipelineState::EPS_WireFrame].Get());
		}
		DrawActors(World->GetActorsRef()[(int)EActorType::EAT_Transparency], TargetFrameResource);
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

	DeviceManager->SignalFence();
	DeviceManager->PresentAndSwapBuffer();
	TargetFrameResource->Fence = DeviceManager->GetCurrentFence();
}

void FRenderer::BuildDescriptorHeaps()
{
	// Build SRVHeap
	D3D12_DESCRIPTOR_HEAP_DESC SRVHeapDesc;
	ZeroMemory(&SRVHeapDesc, sizeof(D3D12_DESCRIPTOR_HEAP_DESC));
	SRVHeapDesc.NumDescriptors = (UINT)FTexture::Textures.size();
	SRVHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	SRVHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	THROW_IF_FAILED(Device->CreateDescriptorHeap(&SRVHeapDesc, IID_PPV_ARGS(SRVHeap.GetAddressOf())));
}

void FRenderer::BuildShaderResources()
{
	//
	// Fill out the heap with actual descriptors.
	//

	for (auto& Item : FTexture::Textures)
	{
		auto TextureBuffer = Item.second->Resource;

		CD3DX12_CPU_DESCRIPTOR_HANDLE SRVHandle(SRVHeap->GetCPUDescriptorHandleForHeapStart());
		SRVHandle.Offset(Item.first, FDXResourceManager::GetInstance()->GetCBVSRVUAVDescriptorSize());

		D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
		srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		srvDesc.Format = TextureBuffer->GetDesc().Format;
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MostDetailedMip = 0;
		srvDesc.Texture2D.MipLevels = TextureBuffer->GetDesc().MipLevels;
		srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;

		Device->CreateShaderResourceView(TextureBuffer.Get(), &srvDesc, SRVHandle);
	}
}

void FRenderer::BuildRootSignature()
{
	CD3DX12_DESCRIPTOR_RANGE TextureTable;
	TextureTable.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);

	CD3DX12_ROOT_PARAMETER RootParameter[4];
	RootParameter[0].InitAsConstantBufferView(0);
	RootParameter[1].InitAsConstantBufferView(1);
	RootParameter[2].InitAsConstantBufferView(2);
	RootParameter[3].InitAsDescriptorTable(1, &TextureTable, D3D12_SHADER_VISIBILITY_PIXEL);

	auto StaticSamplers = FTexture::GetStaticSamplers();

	CD3DX12_ROOT_SIGNATURE_DESC RootSignatureDesc(
		4,
		RootParameter,
		(UINT)StaticSamplers.size(),
		StaticSamplers.data(),
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT
	);

	Microsoft::WRL::ComPtr<ID3DBlob> SerializedRootSignature = nullptr;
	Microsoft::WRL::ComPtr<ID3DBlob> ErrorBlob = nullptr;
	HRESULT HResult = D3D12SerializeRootSignature(
		&RootSignatureDesc,
		D3D_ROOT_SIGNATURE_VERSION_1,
		SerializedRootSignature.GetAddressOf(),
		ErrorBlob.GetAddressOf()
	);

	if (ErrorBlob != nullptr)
	{
		::OutputDebugStringA((char*)ErrorBlob->GetBufferPointer());
	}
	THROW_IF_FAILED(HResult);

	THROW_IF_FAILED(
		Device->CreateRootSignature(
			0,
			SerializedRootSignature->GetBufferPointer(),
			SerializedRootSignature->GetBufferSize(),
			IID_PPV_ARGS(RootSignature.GetAddressOf())
		)
	)
}

void FRenderer::BuildShaderAndInputLayout()
{
	D3D_SHADER_MACRO Defines[] = {
		{"FOG", "1"},
		{NULL, NULL}
	};

	D3D_SHADER_MACRO AlphaTestDefine[] = {
		{"FOG", "1"},
		{"ALPHA_TEST", "1"},
		{NULL, NULL}
	};

	Shaders["ForwardLitVertexShader"] = FDXUtility::CompileShader(
		L"Shaders\\ForwardLitVertexShader.sf",
		nullptr,
		"MainVS",
		"vs_5_1"
	);
	Shaders["ForwardLitPixelShader"] = FDXUtility::CompileShader(
		L"Shaders\\ForwardLitPixelShader.sf",
		Defines,
		"MainPS",
		"ps_5_1"
	);

	Shaders["AlphTestPixelShader"] = FDXUtility::CompileShader(
		L"Shaders\\ForwardLitPixelShader.sf",
		AlphaTestDefine,
		"MainPS",
		"ps_5_1"
	);

	Shaders["BillboardVertexShader"] = FDXUtility::CompileShader(
		L"Shaders\\BillboardVertexShader.sf",
		nullptr,
		"MainVS",
		"vs_5_1"
	);

	Shaders["BillboardGeometryShader"] = FDXUtility::CompileShader(
		L"Shaders\\BillboardGeometryShader.sf",
		nullptr,
		"MainGS",
		"gs_5_1"
	);

	Shaders["BillboardPixelShader"] = FDXUtility::CompileShader(
		L"Shaders\\BillboardPixelShader.sf",
		AlphaTestDefine,
		"MainPS",
		"ps_5_1"
	);

	InputLayouts["Lit"] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
	};

	InputLayouts["Billboard"] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "SIZE", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
	};
}

void FRenderer::BuildPipelineStateObject()
{
	FDXResourceManager* DeviceManager = FDXResourceManager::GetInstance();

	PipelineStateObjects.resize((size_t)EPipelineState::EPS_None);
	D3D12_GRAPHICS_PIPELINE_STATE_DESC ForwardLitPipelineStateDesc;
	//
	// PSO for opaque objects.
	//
	ZeroMemory(&ForwardLitPipelineStateDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
	ForwardLitPipelineStateDesc.InputLayout = { InputLayouts["Lit"].data(), (UINT)InputLayouts["Lit"].size() };
	ForwardLitPipelineStateDesc.pRootSignature = RootSignature.Get();
	ForwardLitPipelineStateDesc.VS =
	{
		reinterpret_cast<BYTE*>(Shaders["ForwardLitVertexShader"]->GetBufferPointer()),
		Shaders["ForwardLitVertexShader"]->GetBufferSize()
	};
	ForwardLitPipelineStateDesc.PS =
	{
		reinterpret_cast<BYTE*>(Shaders["ForwardLitPixelShader"]->GetBufferPointer()),
		Shaders["ForwardLitPixelShader"]->GetBufferSize()
	};
	ForwardLitPipelineStateDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	ForwardLitPipelineStateDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	ForwardLitPipelineStateDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	ForwardLitPipelineStateDesc.SampleMask = UINT_MAX;
	ForwardLitPipelineStateDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	ForwardLitPipelineStateDesc.NumRenderTargets = 1;
	ForwardLitPipelineStateDesc.RTVFormats[0] = DeviceManager->GetBackbufferFormat();
	ForwardLitPipelineStateDesc.SampleDesc.Count = DeviceManager->IsMSAAOn() ? 4 : 1;
	ForwardLitPipelineStateDesc.SampleDesc.Quality = DeviceManager->IsMSAAOn() ? (DeviceManager->GetMSAAQuality_4x() - 1) : 0;
	ForwardLitPipelineStateDesc.DSVFormat = DeviceManager->GetDepthStencilFormat();
	THROW_IF_FAILED(
		Device->CreateGraphicsPipelineState(
			&ForwardLitPipelineStateDesc, IID_PPV_ARGS(&PipelineStateObjects[(int)EPipelineState::EPS_Opaque])
		)
	);


	//
	// PSO for opaque wireframe objects.
	//

	{
		D3D12_GRAPHICS_PIPELINE_STATE_DESC WireFramePipelineStateDesc = ForwardLitPipelineStateDesc;
		WireFramePipelineStateDesc.RasterizerState.FillMode = D3D12_FILL_MODE_WIREFRAME;
		THROW_IF_FAILED(
			Device->CreateGraphicsPipelineState(
				&WireFramePipelineStateDesc, IID_PPV_ARGS(&PipelineStateObjects[(int)EPipelineState::EPS_WireFrame])
			)
		);
	}

	{
		D3D12_GRAPHICS_PIPELINE_STATE_DESC TransparencyPipelineStateDesc = ForwardLitPipelineStateDesc;

		D3D12_RENDER_TARGET_BLEND_DESC BlendDesc;
		BlendDesc.BlendEnable = true;
		BlendDesc.LogicOpEnable = false;
		BlendDesc.SrcBlend = D3D12_BLEND_SRC_ALPHA;
		BlendDesc.DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
		BlendDesc.BlendOp = D3D12_BLEND_OP_ADD;
		BlendDesc.SrcBlendAlpha = D3D12_BLEND_ONE;
		BlendDesc.DestBlendAlpha = D3D12_BLEND_ZERO;
		BlendDesc.BlendOpAlpha = D3D12_BLEND_OP_ADD;
		BlendDesc.LogicOp = D3D12_LOGIC_OP_NOOP;
		BlendDesc.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

		TransparencyPipelineStateDesc.BlendState.RenderTarget[0] = BlendDesc;
		THROW_IF_FAILED(
			Device->CreateGraphicsPipelineState(
				&TransparencyPipelineStateDesc,
				IID_PPV_ARGS(&PipelineStateObjects[(int)EPipelineState::EPS_Transparency])
			)
		);
	}

	{
		D3D12_GRAPHICS_PIPELINE_STATE_DESC AlphaTestPipelineStateDesc = ForwardLitPipelineStateDesc;
		AlphaTestPipelineStateDesc.PS =
		{
			reinterpret_cast<BYTE*>(Shaders["AlphTestPixelShader"]->GetBufferPointer()),
			Shaders["AlphTestPixelShader"]->GetBufferSize()
		};

		AlphaTestPipelineStateDesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
		THROW_IF_FAILED(
			Device->CreateGraphicsPipelineState(
				&AlphaTestPipelineStateDesc,
				IID_PPV_ARGS(&PipelineStateObjects[(int)EPipelineState::EPS_AlphaTest])
			)
		);
	}

	// Billboard
	{
		D3D12_GRAPHICS_PIPELINE_STATE_DESC BillboardPipelineStateDesc = ForwardLitPipelineStateDesc;
		BillboardPipelineStateDesc.VS =
		{
			reinterpret_cast<BYTE*>(Shaders["BillboardVertexShader"]->GetBufferPointer()),
			Shaders["BillboardVertexShader"]->GetBufferSize()
		};
		BillboardPipelineStateDesc.GS =
		{
			reinterpret_cast<BYTE*>(Shaders["BillboardGeometryShader"]->GetBufferPointer()),
			Shaders["BillboardGeometryShader"]->GetBufferSize()
		};
		BillboardPipelineStateDesc.PS =
		{
			reinterpret_cast<BYTE*>(Shaders["BillboardPixelShader"]->GetBufferPointer()),
			Shaders["BillboardPixelShader"]->GetBufferSize()
		};
		BillboardPipelineStateDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT;
		BillboardPipelineStateDesc.InputLayout = { InputLayouts["Billboard"].data(), (UINT)InputLayouts["Billboard"].size() };
		BillboardPipelineStateDesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;

		THROW_IF_FAILED(
			Device->CreateGraphicsPipelineState(
				&BillboardPipelineStateDesc,
				IID_PPV_ARGS(PipelineStateObjects[(int)EPipelineState::EPS_Billboard].GetAddressOf())
			)
		);
	}
}

void FRenderer::DrawActors(const std::vector<AActor*>& DrawTargets, FFrameResource* TargetFrameResource)
{
	int ActorCount = (int)DrawTargets.size();

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
