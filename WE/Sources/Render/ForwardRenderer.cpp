#include "ForwardRenderer.h"
#include "Runtime/World/World.h"

void FForwardRenderer::Render()
{
	Super::Render();

	auto TargetCommandAllocator = TargetFrameResource->CommandAllocator.Get();

	ID3D12Device* Device = DXWindow->GetDevice();
	
	ID3D12CommandQueue* CommandQueue = DXWindow->GetCommandQueue();
	ID3D12GraphicsCommandList* CommandList = DXWindow->GetCommandList();
	ID3D12Fence* Fence = DXWindow->GetFence();
	ID3D12Resource* RenderTarget = DXWindow->GetCurrentBackBuffer();

	ThrowIfFailed(TargetCommandAllocator->Reset());

	if (!bWireFrame)
	{
		ThrowIfFailed(CommandList->Reset(TargetCommandAllocator, PipelineStateObjects["BasePass"].Get()));
	}
	else
	{
		ThrowIfFailed(CommandList->Reset(TargetCommandAllocator, PipelineStateObjects["BasePass_WireFrame"].Get()));
	}

	D3D12_VIEWPORT Viewport = DXWindow->GetViewport();
	D3D12_RECT ScissorRect = DXWindow->GetScissorRect();
	CommandList->RSSetViewports(1, &Viewport);
	CommandList->RSSetScissorRects(1, &ScissorRect);

	D3D12_RESOURCE_BARRIER ResourceBarrier = CD3DX12_RESOURCE_BARRIER::Transition(
		RenderTarget,
		D3D12_RESOURCE_STATE_PRESENT,
		D3D12_RESOURCE_STATE_RENDER_TARGET
	);
	CommandList->ResourceBarrier(1, &ResourceBarrier);

	D3D12_CPU_DESCRIPTOR_HANDLE RenderTargetView = DXWindow->GetCurrentBackBufferView();
	D3D12_CPU_DESCRIPTOR_HANDLE DepthStencilView = DXWindow->GetDepthStencilView();
	CommandList->ClearRenderTargetView(RenderTargetView, Colors::Black, 0, nullptr);
	CommandList->ClearDepthStencilView(DepthStencilView, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);

	CommandList->OMSetRenderTargets(1, &RenderTargetView, true, &DepthStencilView);

	////////////////////////////////////////////////////////////////////////////////

	// Render



	static ComPtr<ID3DBlob> VBCPUBuffer;
	static ComPtr<ID3DBlob> IBCPUBuffer;
	static ComPtr<ID3D12Resource> VBUploader, IBUploader;
	static ComPtr<ID3D12Resource> VBGPUBuffer, IBGPUBuffer = nullptr;
	static D3D12_VERTEX_BUFFER_VIEW VBV;
	static D3D12_INDEX_BUFFER_VIEW IBV;
	if (IBGPUBuffer == nullptr)
	{
		Vertex TV[] =
		{
			Vertex({XMFLOAT3(-0.5f, 0, 0), XMFLOAT4(-0.5f, 0, 0, 1)}),
			Vertex({XMFLOAT3(0, 0.5f, 0), XMFLOAT4(0, 0.5f, 0, 1)}),
			Vertex({XMFLOAT3(0.5f, 0, 0), XMFLOAT4(0.5f, 0, 0, 1)})
		};
		UINT16 TI[] = { 0, 1, 2 };
		UINT VBSize = sizeof(Vertex) * 3;
		UINT IBSize = sizeof(UINT16) * 3;
		D3DCreateBlob(VBSize, &VBCPUBuffer);
		CopyMemory(VBCPUBuffer->GetBufferPointer(), TV, VBSize);
		D3DCreateBlob(IBSize, &IBCPUBuffer);
		CopyMemory(IBCPUBuffer->GetBufferPointer(), TI, IBSize);
		VBGPUBuffer = UDXUtility::CreateDefaultBuffer(
			Device,
			CommandList,
			TV,
			VBSize,
			VBUploader
		);
		IBGPUBuffer = UDXUtility::CreateDefaultBuffer(
			Device,
			CommandList,
			TI,
			IBSize,
			IBUploader
		);
		VBV.BufferLocation = VBGPUBuffer->GetGPUVirtualAddress();
		VBV.StrideInBytes = sizeof(Vertex);
		VBV.SizeInBytes = VBSize;
		IBV.BufferLocation = IBGPUBuffer->GetGPUVirtualAddress();
		IBV.Format = DXGI_FORMAT_R16_UINT;
		IBV.SizeInBytes = IBSize;

	};	
	// Constantbuffer

	ID3D12DescriptorHeap* descriptorHeaps[] = { CBVHeap.Get() };
	CommandList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);

	CommandList->SetGraphicsRootSignature(RootSignature.Get());

	CommandList->IASetVertexBuffers(0, 1, &VBV);
	CommandList->IASetIndexBuffer(&IBV);
	CommandList->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	CommandList->SetGraphicsRootDescriptorTable(0, CBVHeap->GetGPUDescriptorHandleForHeapStart());

	CommandList->DrawIndexedInstanced(3, 1, 0, 0, 0);



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

	DXWindow->SwapRenderTargetBuffers();

	TargetFrameResource->Fence = DXWindow->GetNextFence();
	CommandQueue->Signal(Fence, DXWindow->GetCurrentFence());
}

void FForwardRenderer::BuildDescriptorHeaps()
{
	ID3D12Device* Device = DXWindow->GetDevice();
	D3D12_DESCRIPTOR_HEAP_DESC cbvHeapDesc;
	cbvHeapDesc.NumDescriptors = NumFrameResources;
	cbvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	cbvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	cbvHeapDesc.NodeMask = 0;
	ThrowIfFailed(Device->CreateDescriptorHeap(&cbvHeapDesc,
		IID_PPV_ARGS(&CBVHeap)));
}

void FForwardRenderer::BuildConstantBuffers()
{
	ID3D12Device* Device = DXWindow->GetDevice();

	UINT PassConstantBufferSize = UDXUtility::CalcConstantBufferByteSize(sizeof(FPassConstants));
	for (int i = 0 ; i < NumFrameResources; ++i)
	{
		auto& FrameResource = FrameResources[i];
		D3D12_GPU_VIRTUAL_ADDRESS cbAddress = FrameResource->PassConstantBuffer->Resource()->GetGPUVirtualAddress();
		D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc;
		cbvDesc.BufferLocation = cbAddress;
		cbvDesc.SizeInBytes = PassConstantBufferSize;
		CD3DX12_CPU_DESCRIPTOR_HANDLE Handle = CD3DX12_CPU_DESCRIPTOR_HANDLE(CBVHeap->GetCPUDescriptorHandleForHeapStart());
		Handle.Offset(i, DXWindow->GetCBVSRVUAVDescriptorSize());
		Device->CreateConstantBufferView(
			&cbvDesc,
			Handle
		);
	}
}

void FForwardRenderer::BuildRootSignature()
{
	ID3D12Device* Device = DXWindow->GetDevice();

	CD3DX12_DESCRIPTOR_RANGE ConstantBufferTable0;
	ConstantBufferTable0.Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0);

	CD3DX12_ROOT_PARAMETER RootParameter[1];
	RootParameter[0].InitAsDescriptorTable(1, &ConstantBufferTable0);

	CD3DX12_ROOT_SIGNATURE_DESC RootSignatureDesc(
		1,
		RootParameter,
		0,
		nullptr,
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

void FForwardRenderer::BuildShaderAndInputLayout()
{
	Shaders["BasePassVertexShader"] = UDXUtility::CompileShader(
		L"Shaders\\BasePassVertexShader.sf",
		nullptr,
		"MainVS",
		"vs_5_1"
	);
	Shaders["BasePassPixelShader"] = UDXUtility::CompileShader(
		L"Shaders\\BasePassPixelShader.sf",
		nullptr,
		"MainPS",
		"ps_5_1"
	);

	InputLayout =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
	};
}

void FForwardRenderer::BuildPipelineStateObject()
{
	ID3D12Device* Device = DXWindow->GetDevice();

	D3D12_GRAPHICS_PIPELINE_STATE_DESC BasePassPipelineStateDesc;

	//
	// PSO for opaque objects.
	//
	ZeroMemory(&BasePassPipelineStateDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
	BasePassPipelineStateDesc.InputLayout = { InputLayout.data(), (UINT)InputLayout.size() };
	BasePassPipelineStateDesc.pRootSignature = RootSignature.Get();
	BasePassPipelineStateDesc.VS =
	{
		reinterpret_cast<BYTE*>(Shaders["BasePassVertexShader"]->GetBufferPointer()),
		Shaders["BasePassVertexShader"]->GetBufferSize()
	};
	BasePassPipelineStateDesc.PS =
	{
		reinterpret_cast<BYTE*>(Shaders["BasePassPixelShader"]->GetBufferPointer()),
		Shaders["BasePassPixelShader"]->GetBufferSize()
	};
	BasePassPipelineStateDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	BasePassPipelineStateDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	BasePassPipelineStateDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	BasePassPipelineStateDesc.SampleMask = UINT_MAX;
	BasePassPipelineStateDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	BasePassPipelineStateDesc.NumRenderTargets = 1;
	BasePassPipelineStateDesc.RTVFormats[0] = DXWindow->GetBackBufferFormat();
	BasePassPipelineStateDesc.SampleDesc.Count = DXWindow->Get4xMSAAState() ? 4 : 1;
	BasePassPipelineStateDesc.SampleDesc.Quality = DXWindow->Get4xMSAAState() ? (DXWindow->Get4xMSAAQuality() - 1) : 0;
	BasePassPipelineStateDesc.DSVFormat = DXWindow->GetDepthStencilFormat();
	ThrowIfFailed(
		Device->CreateGraphicsPipelineState(
			&BasePassPipelineStateDesc, IID_PPV_ARGS(&PipelineStateObjects["BasePass"])
		)
	);


	//
	// PSO for opaque wireframe objects.
	//

	D3D12_GRAPHICS_PIPELINE_STATE_DESC BasePassWireFramePipelineStateDesc = BasePassPipelineStateDesc;
	BasePassWireFramePipelineStateDesc.RasterizerState.FillMode = D3D12_FILL_MODE_WIREFRAME;
	ThrowIfFailed(
		Device->CreateGraphicsPipelineState(
			&BasePassWireFramePipelineStateDesc, IID_PPV_ARGS(&PipelineStateObjects["BasePass_WireFrame"])
		)
	);
}