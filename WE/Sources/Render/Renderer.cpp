#include "Renderer.h"

#include "MeshGeometry.h"
#include "Texture.h"
#include "Material.h"
#include "DirectX/DXDeviceManager.h"
#include "Utility/DXUtility.h"
#include "Utility/Timer.h"
#include "Runtime/World/World.h"
//#include "Runtime/Object/ViewCamera.h"

FRenderer::FRenderer()
{
	FDXDeviceManager* DeviceManager = FDXDeviceManager::GetInstance();
	Device = DeviceManager->GetDevicePtr();
	CommandQueue = DeviceManager->GetCommandQueuePtr();
	CommandAllocator = DeviceManager->GetCommandAllocatorPtr();
	CommandList = DeviceManager->GetCommandListPtr();
	Fence = DeviceManager->GetFencePtr();
}

bool FRenderer::Initialize(WWorld* InWorld)
{
	World = InWorld;

	BuildFrameResources();
	BuildDescriptorHeaps();
	BuildShaderResources();
	BuildRootSignature();
	BuildShaderAndInputLayout();
	BuildPipelineStateObject();

	return true;
}

void FRenderer::Render(class UTimer* Timer)
{
	SetTargetFrameResource();

	UpdatePassConstantBuffers(Timer);
	UpdateObjectConstantBuffer();
	UpdateMaterialConstantBuffer();

	auto TargetCommandAllocator = TargetFrameResource->CommandAllocator.Get();

	FDXDeviceManager* DeviceManager = FDXDeviceManager::GetInstance();
	ID3D12Resource* RenderTarget = DeviceManager->GetCurrentBackBufferPtr();

	ThrowIfFailed(TargetCommandAllocator->Reset());
	ThrowIfFailed(CommandList->Reset(TargetCommandAllocator, nullptr));

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
		DrawActors(World->GetActorsRef()[(int)EActorType::EAT_Opaque]);
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
		DrawActors(World->GetActorsRef()[(int)EActorType::EAT_AlphaTest]);
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
		DrawActors(World->GetActorsRef()[(int)EActorType::EAT_Billboard]);
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
		DrawActors(World->GetActorsRef()[(int)EActorType::EAT_Transparency]);
	}

	////////////////////////////////////////////////////////////////////////////////

	ResourceBarrier = CD3DX12_RESOURCE_BARRIER::Transition(
		RenderTarget,
		D3D12_RESOURCE_STATE_RENDER_TARGET,
		D3D12_RESOURCE_STATE_PRESENT
	);
	CommandList->ResourceBarrier(1, &ResourceBarrier);

	ThrowIfFailed(CommandList->Close());

	ID3D12CommandList* CommandLists[] = { CommandList };
	CommandQueue->ExecuteCommandLists(_countof(CommandLists), CommandLists);

	DeviceManager->PresentAndSwapBuffer();
	DeviceManager->SignalFence();
	TargetFrameResource->Fence = DeviceManager->GetCurrentFence();
}

void FRenderer::BuildFrameResources()
{
	for (int i = 0; i < FRAME_RESOURCES_NUM; ++i)
	{
		FrameResources.push_back(
			std::make_unique<FFrameResource>(
				Device,
				1u,
				(UINT)World->GetAllActorsRef().size(),
				(UINT)FMaterial::Materials.size()
			)
		);
	}
}

void FRenderer::BuildDescriptorHeaps()
{
	// Build SRVHeap
	D3D12_DESCRIPTOR_HEAP_DESC SRVHeapDesc;
	ZeroMemory(&SRVHeapDesc, sizeof(D3D12_DESCRIPTOR_HEAP_DESC));
	SRVHeapDesc.NumDescriptors = (UINT)FTexture::Textures.size();
	SRVHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	SRVHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	ThrowIfFailed(Device->CreateDescriptorHeap(&SRVHeapDesc, IID_PPV_ARGS(SRVHeap.GetAddressOf())));
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
		SRVHandle.Offset(Item.first, FDXDeviceManager::GetInstance()->GetCBVSRVUAVDescriptorSize());

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

	ComPtr<ID3DBlob> SerializedRootSignature = nullptr;
	ComPtr<ID3DBlob> ErrorBlob = nullptr;
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
	ThrowIfFailed(HResult);

	ThrowIfFailed(
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

	Shaders["ForwardLitVertexShader"] = UDXUtility::CompileShader(
		L"Shaders\\ForwardLitVertexShader.sf",
		nullptr,
		"MainVS",
		"vs_5_1"
	);
	Shaders["ForwardLitPixelShader"] = UDXUtility::CompileShader(
		L"Shaders\\ForwardLitPixelShader.sf",
		Defines,
		"MainPS",
		"ps_5_1"
	);

	Shaders["AlphTestPixelShader"] = UDXUtility::CompileShader(
		L"Shaders\\ForwardLitPixelShader.sf",
		AlphaTestDefine,
		"MainPS",
		"ps_5_1"
	);

	Shaders["BillboardVertexShader"] = UDXUtility::CompileShader(
		L"Shaders\\BillboardVertexShader.sf",
		nullptr,
		"MainVS",
		"vs_5_1"
	);

	Shaders["BillboardGeometryShader"] = UDXUtility::CompileShader(
		L"Shaders\\BillboardGeometryShader.sf",
		nullptr,
		"MainGS",
		"gs_5_1"
	);

	Shaders["BillboardPixelShader"] = UDXUtility::CompileShader(
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
	FDXDeviceManager* DeviceManager = FDXDeviceManager::GetInstance();

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
	ThrowIfFailed(
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
		ThrowIfFailed(
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
		ThrowIfFailed(
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
		ThrowIfFailed(
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

		ThrowIfFailed(
			Device->CreateGraphicsPipelineState(
				&BillboardPipelineStateDesc,
				IID_PPV_ARGS(PipelineStateObjects[(int)EPipelineState::EPS_Billboard].GetAddressOf())
			)
		);
	}
}

void FRenderer::SetTargetFrameResource()
{
	// Cycle through the circular frame resource array.
	TargetFrameResourceIndex = (TargetFrameResourceIndex + 1) % NumFrameResources;
	TargetFrameResource = FrameResources[TargetFrameResourceIndex].get();

	// Has the GPU finished processing the commands of the current frame resource?
	// If not, wait until the GPU has completed commands up to this fence point.
	if (TargetFrameResource->Fence != 0 && Fence->GetCompletedValue() < TargetFrameResource->Fence)
	{
		HANDLE eventHandle = CreateEventEx(nullptr, nullptr, false, EVENT_ALL_ACCESS);
		ThrowIfFailed(Fence->SetEventOnCompletion(TargetFrameResource->Fence, eventHandle));
		WaitForSingleObject(eventHandle, INFINITE);
		CloseHandle(eventHandle);
	}
}

void FRenderer::UpdatePassConstantBuffers(UTimer* Timer)
{
	FDXDeviceManager* DeviceManager = FDXDeviceManager::GetInstance();

	Camera->UpdateViewMatrix();
	// Build the view matrix.
	XMVECTOR target = XMVectorZero();
	XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

	XMMATRIX view = Camera->GetViewMatrix();
	XMMATRIX InvView = UDXMath::GetInverseMatrix(view);
	XMMATRIX proj = Camera->GetProjMatrix();
	XMMATRIX InvProj = UDXMath::GetInverseMatrix(proj);
	XMMATRIX ViewProj = view * proj;
	XMMATRIX InvViewProj = UDXMath::GetInverseMatrix(ViewProj);

	FPassConstants PassConstants;
	XMStoreFloat4x4(&PassConstants.View, XMMatrixTranspose(view));
	XMStoreFloat4x4(&PassConstants.InvView, XMMatrixTranspose(InvView));
	XMStoreFloat4x4(&PassConstants.Proj, XMMatrixTranspose(proj));
	XMStoreFloat4x4(&PassConstants.InvProj, XMMatrixTranspose(InvProj));
	XMStoreFloat4x4(&PassConstants.ViewProj, XMMatrixTranspose(ViewProj));
	XMStoreFloat4x4(&PassConstants.InvViewProj, XMMatrixTranspose(InvViewProj));
	PassConstants.EyePosW = Camera->GetTranslation();
	D3D12_VIEWPORT Viewport = DeviceManager->GetScreenViewport();
	float Width = static_cast<float>(Viewport.Width);
	float Height = static_cast<float>(Viewport.Height);
	PassConstants.RenderTargetSize = XMFLOAT2(Width, Height);
	PassConstants.InvRenderTargetSize = XMFLOAT2(1.0f / Width, 1.0f / Height);
	PassConstants.NearZ = 1.0f;
	PassConstants.FarZ = 1000.0f;
	PassConstants.TotalTime = Timer->GetTotalTime();
	PassConstants.DeltaTime = Timer->GetDeltaTime();

	PassConstants.AmbientLight = { 0.25f, 0.25f, 0.35f, 1.0f };

	PassConstants.FogColor = XMFLOAT4(Colors::LightSkyBlue);
	PassConstants.FogStart = 200.0f;
	PassConstants.FogRange = 100.0f;

	PassConstants.Lights[0].Direction = { 0.57735f, -0.57735f, 0.57735f };
	PassConstants.Lights[0].Strength = { 1.2f, 1.2f, 1.2f };
	PassConstants.Lights[1].Direction = { -0.57735f, -0.57735f, 0.57735f };
	PassConstants.Lights[1].Strength = { 0.3f, 0.3f, 0.3f };
	PassConstants.Lights[2].Direction = { 0.0f, -0.707f, -0.707f };
	PassConstants.Lights[2].Strength = { 0.15f, 0.15f, 0.15f };

	TargetFrameResource->PassConstantBuffer->CopyData(0, PassConstants);
}

void FRenderer::UpdateObjectConstantBuffer()
{
	const auto& Actors = World->GetAllActorsRef();
	for (auto& Actor : Actors)
	{
		if (Actor->NumFramesDirty > 0)
		{
			FObjectConstants Constants;
			XMStoreFloat4x4(&Constants.World, XMMatrixTranspose(Actor->GetWorldMatrix()));
			XMStoreFloat4x4(&Constants.TexTransform, XMMatrixTranspose(Actor->GetTextureTransformMatrix()));
			TargetFrameResource->ObjectConstantBuffer->CopyData(Actor->ObjectConstantBufferIndex, Constants);
			--Actor->NumFramesDirty;
		}
	}
}

void FRenderer::UpdateMaterialConstantBuffer()
{
	for (auto& Item : FMaterial::Materials)
	{
		FMaterial* Material = Item.second.get();
		if (Material->NumFramesDirty > 0)
		{
			XMMATRIX MaterialTransform = XMLoadFloat4x4(&Material->MatTransform);
			FMaterialConstants MaterialConstants;
			MaterialConstants.DiffuseAlbedo = Material->DiffuseAlbedo;
			MaterialConstants.FresnelR0 = Material->FresnelR0;
			XMStoreFloat4x4(&MaterialConstants.MatTransform, XMMatrixTranspose(MaterialTransform));
			MaterialConstants.Roughness = Material->Roughness;

			TargetFrameResource->MaterialConstantBuffer->CopyData(Material->MatCBIndex, MaterialConstants);
			--Material->NumFramesDirty;
		}
	}
}

void FRenderer::DrawActors(const vector<AActor*>& DrawTargets)
{
	int ActorCount = (int)DrawTargets.size();
	int MaterialCount = (int)FMaterial::Materials.size();

	ID3D12Resource* ObjectConstantBuffer = TargetFrameResource->ObjectConstantBuffer->Resource();
	ID3D12Resource* MaterialConstantBuffer = TargetFrameResource->MaterialConstantBuffer->Resource();

	UINT ObjectConstantBufferByteSize = UDXUtility::CalcConstantBufferByteSize(sizeof(FObjectConstants));
	UINT MaterialConstantBufferByteSize = UDXUtility::CalcConstantBufferByteSize(sizeof(FMaterialConstants));
	UINT CBVSRVUAVDescriptorSize = FDXDeviceManager::GetInstance()->GetCBVSRVUAVDescriptorSize();

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
