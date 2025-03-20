#include "CubeSkyRenderer.h"
#include <DirectXColors.h>
#include "Texture.h"
#include "FrameResource.h"
#include "DirectX/DXResourceManager.h"
#include "MeshGeometry.h"
#include "CubeRenderTarget.h"

FCubeSkyRenderer::FCubeSkyRenderer() :
	mDevice(GetDXResourceManagerPtr()->GetDevicePtr()),
	mSkyTextureCube(GetTextureManager()->GetTextureCube("Snow"))
{
	BuildShaderAndInputLayout();
	BuildDescriptorHeaps();
	BuildDepthStencilBuffer();
	BuildViewMatrix();
	BuildConstantBuffers();
	BuildRootSignature();
	BuildPipelineStateObject();
	BuildDiffuseCubeMap();
}

FCubeSkyRenderer::~FCubeSkyRenderer()
{
}

void FCubeSkyRenderer::Render(ID3D12GraphicsCommandList* CommandList)
{
	CommandList->SetPipelineState(mPipelineState.Get());
	CD3DX12_GPU_DESCRIPTOR_HANDLE SRVHandle(GetTextureManager()->GetTexture2DSRVHeapPtr()->GetGPUDescriptorHandleForHeapStart());
	SRVHandle.Offset(mDiffuseCubeRenderTarget->mTexture->SRVHeapIndex, GetDXResourceManagerPtr()->GetCBVSRVUAVDescriptorSize());
	CommandList->SetGraphicsRootDescriptorTable(5, SRVHandle);

	DrawSphere(CommandList);
}

void FCubeSkyRenderer::BuildShaderAndInputLayout()
{
	mVertexShader = FDXUtility::CompileShader(
		L"Shaders\\CubeSkyVertexShader.sf",
		nullptr,
		"MainVS",
		"vs_5_1"
	);

	mPixelShader = FDXUtility::CompileShader(
		L"Shaders\\CubeSkyPixelShader.sf",
		nullptr,
		"MainPS",
		"ps_5_1"
	);

	mDiffuseMapVertexShader = FDXUtility::CompileShader(
		L"Shaders\\CubeSkyDiffuseMapVertexShader.sf",
		nullptr,
		"MainVS",
		"vs_5_1"
	);

	mDiffuseMapPixelShader = FDXUtility::CompileShader(
		L"Shaders\\CubeSkyDiffuseMapPixelShader.sf",
		nullptr,
		"MainPS",
		"ps_5_1"
	);

	mInputLayout =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
	};
}

void FCubeSkyRenderer::BuildPipelineStateObject()
{
	D3D12_GRAPHICS_PIPELINE_STATE_DESC CubeSkyPSDesc;
	ZeroMemory(&CubeSkyPSDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
	CubeSkyPSDesc.InputLayout = { mInputLayout.data(), (UINT)mInputLayout.size() };
	CubeSkyPSDesc.pRootSignature = GetFrameResourceManager()->GetRootSignaturePtr();
	CubeSkyPSDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	CubeSkyPSDesc.SampleMask = UINT_MAX;
	CubeSkyPSDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	CubeSkyPSDesc.NumRenderTargets = 1;
	CubeSkyPSDesc.RTVFormats[0] = GetDXResourceManagerPtr()->GetBackbufferFormat();
	CubeSkyPSDesc.SampleDesc.Count = GetDXResourceManagerPtr()->IsMSAAOn() ? 4 : 1;
	CubeSkyPSDesc.SampleDesc.Quality = GetDXResourceManagerPtr()->IsMSAAOn() ? (GetDXResourceManagerPtr()->GetMSAAQuality_4x() - 1) : 0;
	CubeSkyPSDesc.DSVFormat = GetDXResourceManagerPtr()->GetDepthStencilFormat();
	CubeSkyPSDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	CubeSkyPSDesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
	CubeSkyPSDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	CubeSkyPSDesc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
	CubeSkyPSDesc.VS =
	{
		reinterpret_cast<BYTE*>(mVertexShader->GetBufferPointer()),
		mVertexShader->GetBufferSize()
	};
	CubeSkyPSDesc.PS =
	{
		reinterpret_cast<BYTE*>(mPixelShader->GetBufferPointer()),
		mPixelShader->GetBufferSize()
	};
	THROW_IF_FAILED(
		GetDXResourceManagerPtr()->GetDevicePtr()->CreateGraphicsPipelineState(
			&CubeSkyPSDesc,
			IID_PPV_ARGS(mPipelineState.GetAddressOf())
		)
	);
	D3D12_GRAPHICS_PIPELINE_STATE_DESC DiffuseCubeSkyPSDesc = CubeSkyPSDesc;
	DiffuseCubeSkyPSDesc.pRootSignature = mRootSignature.Get();
	DiffuseCubeSkyPSDesc.VS =
	{
		reinterpret_cast<BYTE*>(mDiffuseMapVertexShader->GetBufferPointer()),
		mDiffuseMapVertexShader->GetBufferSize()
	};
	DiffuseCubeSkyPSDesc.PS =
	{
		reinterpret_cast<BYTE*>(mDiffuseMapPixelShader->GetBufferPointer()),
		mDiffuseMapPixelShader->GetBufferSize()
	};
	THROW_IF_FAILED(
		GetDXResourceManagerPtr()->GetDevicePtr()->CreateGraphicsPipelineState(
			&DiffuseCubeSkyPSDesc,
			IID_PPV_ARGS(mDiffuseCubePipelineState.GetAddressOf())
		)
	);
}

void FCubeSkyRenderer::BuildDiffuseCubeMap()
{

	mDiffuseCubeRenderTarget = std::make_unique<FCubeRenderTarget>(
		"SkyDiffuse",
		(UINT)mSkyTextureCube->Resource->GetDesc().Width,
		(UINT)mSkyTextureCube->Resource->GetDesc().Height,
		mDiffuseTextureCubeFormat
	);
	FDXResourceManager* DXManager = GetDXResourceManagerPtr();
	FTextureManager* TexManager = GetTextureManager();
	ID3D12DescriptorHeap* SRVHeap = TexManager->GetTexture2DSRVHeapPtr();
	UINT mTextureCubeHeapIndex = mDiffuseCubeRenderTarget->mTexture->SRVHeapIndex;
	UINT CBVSRVUAVDescriptorSize = DXManager->GetCBVSRVUAVDescriptorSize();
	CD3DX12_CPU_DESCRIPTOR_HANDLE SRVCPUHandle = CD3DX12_CPU_DESCRIPTOR_HANDLE(
		SRVHeap->GetCPUDescriptorHandleForHeapStart(),
		mTextureCubeHeapIndex,
		CBVSRVUAVDescriptorSize
		);
	CD3DX12_GPU_DESCRIPTOR_HANDLE SRVGPUHandle = CD3DX12_GPU_DESCRIPTOR_HANDLE(
		SRVHeap->GetGPUDescriptorHandleForHeapStart(),
		mTextureCubeHeapIndex,
		CBVSRVUAVDescriptorSize
	);
	CD3DX12_CPU_DESCRIPTOR_HANDLE RTVCPUHandle[6];
	UINT RTVDescriptorSize = DXManager->GetRTVDescriptorSize();
	for (int i = 0; i < 6; ++i)
	{
		RTVCPUHandle[i] = CD3DX12_CPU_DESCRIPTOR_HANDLE(mRTVHeap->GetCPUDescriptorHandleForHeapStart(), i, RTVDescriptorSize);
	}
	mDiffuseCubeRenderTarget->BuildDescriptors(
		SRVCPUHandle,
		SRVGPUHandle,
		RTVCPUHandle
	);

	DXManager->FlushAndExecuteCommand(&FCubeSkyRenderer::RenderDiffuseMap, this);
}

void FCubeSkyRenderer::BuildDescriptorHeaps()
{
	// RTVHeap
	D3D12_DESCRIPTOR_HEAP_DESC RTVHeapDesc;
	RTVHeapDesc.NumDescriptors = 6;
	RTVHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	RTVHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	RTVHeapDesc.NodeMask = 0;
	THROW_IF_FAILED(
		mDevice->CreateDescriptorHeap(
			&RTVHeapDesc, 
			IID_PPV_ARGS(mRTVHeap.GetAddressOf())
		)
	);

	// DSVHeap
	D3D12_DESCRIPTOR_HEAP_DESC DSVHeapDesc;
	DSVHeapDesc.NumDescriptors = 1;
	DSVHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	DSVHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	DSVHeapDesc.NodeMask = 0;
	THROW_IF_FAILED(
		mDevice->CreateDescriptorHeap(
			&DSVHeapDesc,
			IID_PPV_ARGS(mDSVHeap.GetAddressOf())
		)
	);
}

void FCubeSkyRenderer::BuildDepthStencilBuffer()
{
	D3D12_RESOURCE_DESC depthStencilDesc;
	ZeroMemory(&depthStencilDesc, sizeof(D3D12_RESOURCE_DESC));
	depthStencilDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	depthStencilDesc.Alignment = 0;
	depthStencilDesc.Width = mSkyTextureCube->Resource->GetDesc().Width;
	depthStencilDesc.Height = mSkyTextureCube->Resource->GetDesc().Height;
	depthStencilDesc.DepthOrArraySize = 1;
	depthStencilDesc.MipLevels = 1;
	depthStencilDesc.Format = mDepthStencilFormat;
	depthStencilDesc.SampleDesc.Count = 1;
	depthStencilDesc.SampleDesc.Quality = 0;
	depthStencilDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN; 
	depthStencilDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

	D3D12_CLEAR_VALUE optClear;
	optClear.Format = mDepthStencilFormat;
	optClear.DepthStencil.Depth = 1.0f;
	optClear.DepthStencil.Stencil = 0;
	D3D12_HEAP_PROPERTIES DefaultHeapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
	THROW_IF_FAILED(
		mDevice->CreateCommittedResource(
			&DefaultHeapProperties,
			D3D12_HEAP_FLAG_NONE,
			&depthStencilDesc,
			D3D12_RESOURCE_STATE_COMMON,
			&optClear,
			IID_PPV_ARGS(mDiffuseDepthStencilResource.GetAddressOf())
		)
	);

	mDevice->CreateDepthStencilView(
		mDiffuseDepthStencilResource.Get(),
		nullptr,
		mDSVHeap->GetCPUDescriptorHandleForHeapStart()
	);	
}

void FCubeSkyRenderer::BuildViewMatrix()
{
	// Generate the cube map about the given position.
	XMFLOAT3 Position(0.0f, 0.0f, 0.0f);

	// Look along each coordinate axis.
	XMFLOAT3 targets[6] =
	{
		XMFLOAT3(1.0f, 0.0f, 0.0f), // +X
		XMFLOAT3(-1.0f, 0.0f, 0.0f), // -X
		XMFLOAT3(0.0f, 1.0f, 0.0f), // +Y
		XMFLOAT3(0.0f, -1.0f, 0.0f), // -Y
		XMFLOAT3(0.0f, 0.0f, 1.0f), // +Z
		XMFLOAT3(0.0f, 0.0f, -1.0f)  // -Z
	};

	// Use world up vector (0,1,0) for all directions except +Y/-Y.  In these cases, we
	// are looking down +Y or -Y, so we need a different "up" vector.
	XMFLOAT3 ups[6] =
	{
		XMFLOAT3(0.0f, 1.0f, 0.0f),  // +X
		XMFLOAT3(0.0f, 1.0f, 0.0f),  // -X
		XMFLOAT3(0.0f, 0.0f, -1.0f), // +Y
		XMFLOAT3(0.0f, 0.0f, +1.0f), // -Y
		XMFLOAT3(0.0f, 1.0f, 0.0f),	 // +Z
		XMFLOAT3(0.0f, 1.0f, 0.0f)	 // -Z
	};

	for (int i = 0; i < 6; ++i)
	{
		mCubeViews[i] = FDXMath::CalcViewMatrix(targets[i], ups[i], Position);
	}
}

void FCubeSkyRenderer::BuildConstantBuffers()
{
	mTempCB = std::make_unique <TUploadBuffer<FTempCB>>(
		GetDXResourceManagerPtr()->GetDevicePtr(),
		6,
		true
	);

	
	XMMATRIX P = XMMatrixPerspectiveFovLH(FDXMath::Pi / 2, 1.0f, 0.1f, 10.0f);
	for (int i = 0; i < 6; ++i)
	{
		FTempCB CB;
		XMMATRIX V = XMLoadFloat4x4(&mCubeViews[i]);
		XMMATRIX VP = XMMatrixMultiply(V, P);
		XMStoreFloat4x4(&CB.View, XMMatrixTranspose(V));
		XMStoreFloat4x4(&CB.Proj, XMMatrixTranspose(P));
		XMStoreFloat4x4(&CB.ViewProj, XMMatrixTranspose(VP));
		mTempCB->CopyData(i, CB);
	}
}

void FCubeSkyRenderer::BuildRootSignature()
{
	CD3DX12_DESCRIPTOR_RANGE CubeTextureTable;
	CubeTextureTable.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 0);

	// Tip: 자주 사용되는 것일수록 작은 인덱스에 보관하는게 퍼포먼스가 좋음
	constexpr UINT ROOT_PARAMETERs_NUM = 2;
	CD3DX12_ROOT_PARAMETER RootParameter[ROOT_PARAMETERs_NUM];
	RootParameter[0].InitAsConstantBufferView(0);	
	RootParameter[1].InitAsDescriptorTable(1, &CubeTextureTable, D3D12_SHADER_VISIBILITY_PIXEL);	// CubeTextureTable

	auto StaticSamplers = FTexture::GetStaticSamplers();

	CD3DX12_ROOT_SIGNATURE_DESC RootSignatureDesc(
		ROOT_PARAMETERs_NUM,
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

	ID3D12Device* Device = GetDXResourceManagerPtr()->GetDevicePtr();
	THROW_IF_FAILED(
		Device->CreateRootSignature(
			0,
			SerializedRootSignature->GetBufferPointer(),
			SerializedRootSignature->GetBufferSize(),
			IID_PPV_ARGS(mRootSignature.GetAddressOf())
		)
	)
}

void FCubeSkyRenderer::RenderDiffuseMap(ID3D12Device* Device, ID3D12GraphicsCommandList* CommandList)
{
	D3D12_RESOURCE_BARRIER ResourceBarrier = CD3DX12_RESOURCE_BARRIER::Transition(
		mDiffuseDepthStencilResource.Get(),
		D3D12_RESOURCE_STATE_COMMON,
		D3D12_RESOURCE_STATE_DEPTH_WRITE
	);
	CommandList->ResourceBarrier(
		1,
		&ResourceBarrier
	);

	ResourceBarrier = CD3DX12_RESOURCE_BARRIER::Transition(
		mDiffuseCubeRenderTarget->Resource(),
		D3D12_RESOURCE_STATE_GENERIC_READ,
		D3D12_RESOURCE_STATE_RENDER_TARGET
	);
	CommandList->ResourceBarrier(
		1,
		&ResourceBarrier
	);

	D3D12_VIEWPORT Viewport = mDiffuseCubeRenderTarget->Viewport();
	D3D12_RECT ScissorRect = mDiffuseCubeRenderTarget->ScissorRect();
	CommandList->RSSetViewports(1, &Viewport);
	CommandList->RSSetScissorRects(1, &ScissorRect);

	CommandList->SetPipelineState(mDiffuseCubePipelineState.Get());

	CommandList->SetGraphicsRootSignature(mRootSignature.Get());

	ID3D12DescriptorHeap* SRVHeap = GetTextureManager()->GetTexture2DSRVHeapPtr();
	ID3D12DescriptorHeap* descriptorHeaps[] = { SRVHeap };
	CommandList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);

	CD3DX12_GPU_DESCRIPTOR_HANDLE SRVHandle(SRVHeap->GetGPUDescriptorHandleForHeapStart());
	SRVHandle.Offset(mSkyTextureCube->SRVHeapIndex, GetDXResourceManagerPtr()->GetCBVSRVUAVDescriptorSize());
	CommandList->SetGraphicsRootDescriptorTable(1, SRVHandle);
	for (int i = 0; i < 6; ++i)
	{
		D3D12_CPU_DESCRIPTOR_HANDLE RTV = mDiffuseCubeRenderTarget->Rtv(i);
		D3D12_CPU_DESCRIPTOR_HANDLE DSV = mDSVHeap->GetCPUDescriptorHandleForHeapStart();
		CommandList->ClearRenderTargetView(
			RTV,
			DirectX::Colors::LightBlue,
			0,
			nullptr
		);
		CommandList->ClearDepthStencilView(
			DSV,
			D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL,
			1.0f,
			0,
			0,
			nullptr
		);

		CommandList->OMSetRenderTargets(1, &RTV, true, &DSV);
		ID3D12Resource* TempCB = mTempCB->Resource();
		UINT CBSize = FDXUtility::CalcConstantBufferByteSize(sizeof(FTempCB));
		D3D12_GPU_VIRTUAL_ADDRESS CBAddress = TempCB->GetGPUVirtualAddress() + (i * CBSize);
		CommandList->SetGraphicsRootConstantBufferView(0, CBAddress);
		
		// TODO: Draw
		DrawSphere(CommandList);
		//

	}

	ResourceBarrier = CD3DX12_RESOURCE_BARRIER::Transition(
		mDiffuseCubeRenderTarget->Resource(),
		D3D12_RESOURCE_STATE_RENDER_TARGET,
		D3D12_RESOURCE_STATE_GENERIC_READ		
	);
	CommandList->ResourceBarrier(
		1,
		&ResourceBarrier
	);


	ResourceBarrier = CD3DX12_RESOURCE_BARRIER::Transition(
		mDiffuseDepthStencilResource.Get(),
		D3D12_RESOURCE_STATE_DEPTH_WRITE,
		D3D12_RESOURCE_STATE_COMMON
	);
	CommandList->ResourceBarrier(
		1,
		&ResourceBarrier
	);
}

void FCubeSkyRenderer::DrawSphere(ID3D12GraphicsCommandList* CommandList)
{
	FMeshGeometry* Sphere = GetMeshGeometryManager()->GetMeshGeometry(EMGT_Sphere);
	D3D12_VERTEX_BUFFER_VIEW VertexBufferView = Sphere->VertexBufferView();
	D3D12_INDEX_BUFFER_VIEW IndexBufferView = Sphere->IndexBufferView();

	CommandList->IASetVertexBuffers(0, 1, &VertexBufferView);
	CommandList->IASetIndexBuffer(&IndexBufferView);
	CommandList->IASetPrimitiveTopology(Sphere->PrimitiveType);

	CommandList->DrawIndexedInstanced(
		Sphere->DrawArgs[0].IndexCount,
		1,
		Sphere->DrawArgs[0].StartIndexLocation,
		Sphere->DrawArgs[0].BaseVertexLocation,
		0
	);
}
