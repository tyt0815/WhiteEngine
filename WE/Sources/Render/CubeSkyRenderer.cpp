#include "CubeSkyRenderer.h"
#include <DirectXColors.h>
#include "Texture.h"
#include "FrameResource.h"
#include "DirectX/DXResourceManager.h"
#include "MeshGeometry.h"
#include "CubeRenderTarget.h"
#include "ShapeDrawer.h"
#include "GameFramework/Object/World/World.h"
#include "GameFramework/Object/Component/CameraComponent.h"

FCubeSkyIrradianceMapRenderer::FCubeSkyIrradianceMapRenderer(FCubeRenderTarget* CubeRenderTarget):
	mCubeRenderTarget(CubeRenderTarget)
{
	// Build CB
	mTempCB = std::make_unique <TUploadBuffer<FConstantBuffers>>(
		GetDXResourceManagerPtr()->GetDevicePtr(),
		6,
		true
	);

	std::array<DirectX::XMFLOAT4X4, 6> CubeViews = FCubeRenderTarget::GetCubeMapViews();
	XMMATRIX P = XMMatrixPerspectiveFovLH(FDXMath::Pi / 2, 1.0f, 0.1f, 10.0f);
	for (int i = 0; i < 6; ++i)
	{
		FConstantBuffers CB;
		XMMATRIX V = XMLoadFloat4x4(&CubeViews[i]);
		XMMATRIX VP = XMMatrixMultiply(V, P);
		XMStoreFloat4x4(&CB.View, XMMatrixTranspose(V));
		XMStoreFloat4x4(&CB.Proj, XMMatrixTranspose(P));
		XMStoreFloat4x4(&CB.ViewProj, XMMatrixTranspose(VP));
		mTempCB->CopyData(i, CB);
	}

	// Build RootSignature
	CD3DX12_DESCRIPTOR_RANGE CubeTextureTable;
	CubeTextureTable.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 0);

	// Tip: 자주 사용되는 것일수록 작은 인덱스에 보관하는게 퍼포먼스가 좋음
	constexpr UINT ROOT_PARAMETERs_NUM = 2;
	CD3DX12_ROOT_PARAMETER RootParameter[ROOT_PARAMETERs_NUM];
	RootParameter[0].InitAsConstantBufferView(0);
	RootParameter[1].InitAsDescriptorTable(1, &CubeTextureTable, D3D12_SHADER_VISIBILITY_PIXEL);	// CubeTextureTable

	FDXUtility::BuildRootSignature(
		RootParameter,
		ROOT_PARAMETERs_NUM,
		mRootSignature.GetAddressOf()
	);

	// Compile Shader
	mVertexShader = FDXUtility::CompileShader(
		L"Shaders\\SkyIrradianceCubeMapVertexShader.sf",
		nullptr,
		"MainVS",
		"vs_5_1"
	);

	mPixelShader = FDXUtility::CompileShader(
		L"Shaders\\SkyIrradianceCubeMapPixelShader.sf",
		nullptr,
		"MainPS",
		"ps_5_1"
	);

	// Create PipelineState Object
	D3D12_GRAPHICS_PIPELINE_STATE_DESC DiffuseCubeSkyPSDesc;
	ZeroMemory(&DiffuseCubeSkyPSDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
	auto mInputLayout = GetDrawingSphereInputLayouts();
	DiffuseCubeSkyPSDesc.InputLayout = { mInputLayout.data(), (UINT)mInputLayout.size() };
	DiffuseCubeSkyPSDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	DiffuseCubeSkyPSDesc.SampleMask = UINT_MAX;
	DiffuseCubeSkyPSDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	DiffuseCubeSkyPSDesc.NumRenderTargets = 1;
	DiffuseCubeSkyPSDesc.RTVFormats[0] = GetDXResourceManagerPtr()->GetBackbufferFormat();
	DiffuseCubeSkyPSDesc.SampleDesc.Count = GetDXResourceManagerPtr()->IsMSAAOn() ? 4 : 1;
	DiffuseCubeSkyPSDesc.SampleDesc.Quality = GetDXResourceManagerPtr()->IsMSAAOn() ? (GetDXResourceManagerPtr()->GetMSAAQuality_4x() - 1) : 0;
	DiffuseCubeSkyPSDesc.DSVFormat = GetDXResourceManagerPtr()->GetDepthStencilFormat();
	DiffuseCubeSkyPSDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	DiffuseCubeSkyPSDesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
	DiffuseCubeSkyPSDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	DiffuseCubeSkyPSDesc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
	DiffuseCubeSkyPSDesc.pRootSignature = mRootSignature.Get();
	DiffuseCubeSkyPSDesc.VS =
	{
		reinterpret_cast<BYTE*>(mVertexShader->GetBufferPointer()),
		mVertexShader->GetBufferSize()
	};
	DiffuseCubeSkyPSDesc.PS =
	{
		reinterpret_cast<BYTE*>(mPixelShader->GetBufferPointer()),
		mPixelShader->GetBufferSize()
	};
	THROW_IF_FAILED(
		GetDXResourceManagerPtr()->GetDevicePtr()->CreateGraphicsPipelineState(
			&DiffuseCubeSkyPSDesc,
			IID_PPV_ARGS(mPipelineState.GetAddressOf())
		)
	);
}

void FCubeSkyIrradianceMapRenderer::Render(FTexture* SkyTextureCube)
{
	mSkyTextureCube = SkyTextureCube;
	GetDXResourceManagerPtr()->ExecuteAndFlushCommand(&FCubeSkyIrradianceMapRenderer::Internal_Render, this);
}

void FCubeSkyIrradianceMapRenderer::Internal_Render(ID3D12Device* Device, ID3D12GraphicsCommandList* CommandList)
{
	D3D12_RESOURCE_BARRIER ResourceBarrier = CD3DX12_RESOURCE_BARRIER::Transition(
		mCubeRenderTarget->GetDepthStencilResource(),
		D3D12_RESOURCE_STATE_COMMON,
		D3D12_RESOURCE_STATE_DEPTH_WRITE
	);
	CommandList->ResourceBarrier(
		1,
		&ResourceBarrier
	);

	ResourceBarrier = CD3DX12_RESOURCE_BARRIER::Transition(
		mCubeRenderTarget->GetCubeMapResource(),
		D3D12_RESOURCE_STATE_GENERIC_READ,
		D3D12_RESOURCE_STATE_RENDER_TARGET
	);
	CommandList->ResourceBarrier(
		1,
		&ResourceBarrier
	);

	D3D12_VIEWPORT Viewport = mCubeRenderTarget->GetViewport();
	D3D12_RECT ScissorRect = mCubeRenderTarget->GetScissorRect();
	CommandList->RSSetViewports(1, &Viewport);
	CommandList->RSSetScissorRects(1, &ScissorRect);

	CommandList->SetPipelineState(mPipelineState.Get());

	CommandList->SetGraphicsRootSignature(mRootSignature.Get());

	ID3D12DescriptorHeap* SRVHeap = GetTextureManager()->GetSRVHeapPtr();
	ID3D12DescriptorHeap* descriptorHeaps[] = { SRVHeap };
	CommandList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);
	CommandList->SetGraphicsRootDescriptorTable(1, GetTextureManager()->GetTextureCubeGPUDescriptorHandle(mSkyTextureCube->SRVHeapIndex));
	for (int i = 0; i < 6; ++i)
	{
		for (UINT j = 0; j < mCubeRenderTarget->GetMipLevels(); ++j)
		{
			D3D12_CPU_DESCRIPTOR_HANDLE RTV = mCubeRenderTarget->GetRTV(i, j);
			D3D12_CPU_DESCRIPTOR_HANDLE DSV = mCubeRenderTarget->GetDSVHeap()->GetCPUDescriptorHandleForHeapStart();
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
			UINT CBSize = FDXUtility::CalcConstantBufferByteSize(sizeof(FConstantBuffers));
			D3D12_GPU_VIRTUAL_ADDRESS CBAddress = TempCB->GetGPUVirtualAddress() + (i * CBSize);
			CommandList->SetGraphicsRootConstantBufferView(0, CBAddress);

			DrawSphere(CommandList);
		}
	}

	ResourceBarrier = CD3DX12_RESOURCE_BARRIER::Transition(
		mCubeRenderTarget->GetCubeMapResource(),
		D3D12_RESOURCE_STATE_RENDER_TARGET,
		D3D12_RESOURCE_STATE_GENERIC_READ
	);
	CommandList->ResourceBarrier(
		1,
		&ResourceBarrier
	);


	ResourceBarrier = CD3DX12_RESOURCE_BARRIER::Transition(
		mCubeRenderTarget->GetDepthStencilResource(),
		D3D12_RESOURCE_STATE_DEPTH_WRITE,
		D3D12_RESOURCE_STATE_COMMON
	);
	CommandList->ResourceBarrier(
		1,
		&ResourceBarrier
	);
}

FCubeSkyRenderer::FCubeSkyRenderer(std::string SkyCubeMapName):
	mSkyTextureCube(GetTextureManager()->GetTextureCube(SkyCubeMapName))
{
	BuildCBs();
	BuildRootSignature();
	BuildShaderAndInputLayout();
	BuildPipelineStateObject();
	CraeteIrradianceMap();
}

void FCubeSkyRenderer::Render(ID3D12GraphicsCommandList* CommandList)
{
	UpdateCBs();
	CommandList->SetPipelineState(mPipelineState.Get());
	CommandList->SetGraphicsRootSignature(mRootSignature.Get());
	CommandList->SetGraphicsRootConstantBufferView(0, mCB->Resource()->GetGPUVirtualAddress());
	CommandList->SetGraphicsRootDescriptorTable(1, GetTextureManager()->GetTextureCubeGPUDescriptorHandle(mSkyTextureCube->SRVHeapIndex));

	DrawSphere(CommandList);
}

void FCubeSkyRenderer::BuildShaderAndInputLayout()
{
	mVertexShader = FDXUtility::CompileShader(
		L"Shaders\\SkyCubeMapVertexShader.sf",
		nullptr,
		"MainVS",
		"vs_5_1"
	);

	mPixelShader = FDXUtility::CompileShader(
		L"Shaders\\SkyCubeMapPixelShader.sf",
		nullptr,
		"MainPS",
		"ps_5_1"
	);

	mInputLayout =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
	};
}

void FCubeSkyRenderer::BuildPipelineStateObject()
{
	D3D12_GRAPHICS_PIPELINE_STATE_DESC CubeSkyPSDesc;
	ZeroMemory(&CubeSkyPSDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
	CubeSkyPSDesc.InputLayout = { mInputLayout.data(), (UINT)mInputLayout.size() };
	CubeSkyPSDesc.pRootSignature = mRootSignature.Get();
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
	
}

void FCubeSkyRenderer::BuildCBs()
{
	mCB = std::make_unique<TUploadBuffer<FConstantBuffers>>(
		GetDXResourceManagerPtr()->GetDevicePtr(),
		1,
		true
	);
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

	FDXUtility::BuildRootSignature(
		RootParameter,
		ROOT_PARAMETERs_NUM,
		mRootSignature.GetAddressOf()
	);
}

void FCubeSkyRenderer::CraeteIrradianceMap()
{
	mSkyIrradianceMapName = mSkyTextureCube->Name + "_Irradiance";
	mSkyIrradianceMapRenderTarget = std::make_unique<FCubeRenderTarget>(
		mSkyIrradianceMapName,
		(UINT)mSkyTextureCube->Resource->GetDesc().Width,
		(UINT)mSkyTextureCube->Resource->GetDesc().Height
	);
	SkyIrradianceCubeMapSRVHeapIndex = mSkyIrradianceMapRenderTarget->mTexture->SRVHeapIndex;
	FCubeSkyIrradianceMapRenderer IrradianceMapRenderer(mSkyIrradianceMapRenderTarget.get());
	IrradianceMapRenderer.Render(mSkyTextureCube);
}

void FCubeSkyRenderer::UpdateCBs()
{
	WCameraComponent* Camera = GetWorld()->GetPlayerCamera();
	FConstantBuffers CB;
	CB.gEyePosW = Camera->GetLocation();
	XMFLOAT4X4 View = Camera->GetViewMatrix();
	XMFLOAT4X4 Proj = Camera->GetProjMatrix();
	XMMATRIX V = XMLoadFloat4x4(&View);
	XMMATRIX P = XMLoadFloat4x4(&Proj);
	XMMATRIX VP = V * P;
	XMStoreFloat4x4(&CB.gViewProj, XMMatrixTranspose(VP));
	mCB->CopyData(0, CB);
}
