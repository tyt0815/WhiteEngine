#include "CubeSkyRenderer.h"
#include <DirectXColors.h>
#include "Texture.h"
#include "DirectX/DXResourceManager.h"
#include "MeshGeometry.h"
#include "RenderTarget.h"
#include "CubeRenderTarget.h"
#include "ShapeDrawer.h"
#include "GameFramework/Object/World/World.h"
#include "GameFramework/Object/Component/CameraComponent.h"

FCubeSkyIrradianceMapRenderer::FCubeSkyIrradianceMapRenderer(FCubeRenderTarget* CubeRenderTarget):
	mCubeRenderTarget(CubeRenderTarget)
{
	BuildCB();

	BuildRootSignature();

	BuildShaders();

	BuildPipelineState();
}

void FCubeSkyIrradianceMapRenderer::Render(FTexture* SkyTextureCube)
{
	mSkyTextureCube = SkyTextureCube;

	UpdateCBs();

	GetDXResourceManagerPtr()->ExecuteAndFlushCommand(&FCubeSkyIrradianceMapRenderer::Internal_Render, this);
}

void FCubeSkyIrradianceMapRenderer::UpdateCBs()
{
	// UpdateCB
	std::array<DirectX::XMFLOAT4X4, 6> CubeViews = FCubeRenderTarget::GetCubeMapViews();
	XMMATRIX P = XMMatrixPerspectiveFovLH(FDXMath::Pi / 2, 1.0f, 0.1f, 10.0f);
	for (int i = 0; i < 6; ++i)
	{
		FConstantBuffers CB;
		XMMATRIX V = XMLoadFloat4x4(&CubeViews[i]);
		XMMATRIX VP = XMMatrixMultiply(V, P);
		XMStoreFloat4x4(&CB.ViewProj, XMMatrixTranspose(VP));
		CB.SkyCubeMapIndex = mSkyTextureCube->SRVHeapIndex;
		mTempCB->CopyData(i, CB);
	}
}

void FCubeSkyIrradianceMapRenderer::BuildPipelineState()
{
	// Create PipelineState Object
	D3D12_GRAPHICS_PIPELINE_STATE_DESC DiffuseCubeSkyPSDesc;
	ZeroMemory(&DiffuseCubeSkyPSDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
	auto mInputLayout = GetDrawingSphereInputLayouts();
	DiffuseCubeSkyPSDesc.InputLayout = { mInputLayout.data(), (UINT)mInputLayout.size() };
	DiffuseCubeSkyPSDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	DiffuseCubeSkyPSDesc.SampleMask = UINT_MAX;
	DiffuseCubeSkyPSDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	DiffuseCubeSkyPSDesc.NumRenderTargets = 1;
	DiffuseCubeSkyPSDesc.RTVFormats[0] = mCubeRenderTarget->GetFormat();
	DiffuseCubeSkyPSDesc.SampleDesc.Count = 1;
	DiffuseCubeSkyPSDesc.SampleDesc.Quality = 0;
	DiffuseCubeSkyPSDesc.DSVFormat = mCubeRenderTarget->GetDepthStencilFormat();
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

void FCubeSkyIrradianceMapRenderer::BuildShaders()
{
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
}

void FCubeSkyIrradianceMapRenderer::BuildRootSignature()
{
	// Build RootSignature
	D3D12_DESCRIPTOR_RANGE CubeTextureTable = GetTextureManager()->GetTextureCubeDescriptorRange();

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

void FCubeSkyIrradianceMapRenderer::BuildCB()
{
	// Build CB
	mTempCB = std::make_unique <TUploadBuffer<FConstantBuffers>>(
		GetDXResourceManagerPtr()->GetDevicePtr(),
		6,
		true
	);
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
	CommandList->SetGraphicsRootDescriptorTable(1, GetTextureManager()->GetTextureCubeGPUSRVForHeapStart());
	for (int i = 0; i < 6; ++i)
	{
		for (UINT j = 0; j < mCubeRenderTarget->GetMipLevels(); ++j)
		{
			D3D12_CPU_DESCRIPTOR_HANDLE RTV = mCubeRenderTarget->GetRTV(i, j);
			D3D12_CPU_DESCRIPTOR_HANDLE DSV = mCubeRenderTarget->GetDSVHeap()->GetCPUDescriptorHandleForHeapStart();
			CommandList->ClearRenderTargetView(
				RTV,
				DirectX::Colors::Black,
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

FPreFilteredSkyCubeMapRenderer::FPreFilteredSkyCubeMapRenderer(FCubeRenderTarget* CubeRenderTarget) :
	mCubeRenderTarget(CubeRenderTarget)
{
	BuildCB();
	BuildRootSignature();
	BuildShaders();
	BuildPipelineState();
}

void FPreFilteredSkyCubeMapRenderer::Render(FTexture* SkyTextureCube)
{
	mSkyTextureCube = SkyTextureCube;
	UpdateCBs();
	GetDXResourceManagerPtr()->ExecuteAndFlushCommand(&FPreFilteredSkyCubeMapRenderer::Internal_Render, this);
}

void FPreFilteredSkyCubeMapRenderer::BuildCB()
{
	// Build CB
	mCB = std::make_unique <TUploadBuffer<FConstantBuffers>>(
		GetDXResourceManagerPtr()->GetDevicePtr(),
		6 * mCubeRenderTarget->GetMipLevels(),
		true
	);
}

void FPreFilteredSkyCubeMapRenderer::BuildRootSignature()
{
	// Build RootSignature
	D3D12_DESCRIPTOR_RANGE CubeTextureTable = GetTextureManager()->GetTextureCubeDescriptorRange();

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

void FPreFilteredSkyCubeMapRenderer::BuildShaders()
{
	// Compile Shader
	mVertexShader = FDXUtility::CompileShader(
		L"Shaders\\PreFilteredSkyCubeMapVertexShader.sf",
		nullptr,
		"MainVS",
		"vs_5_1"
	);

	mPixelShader = FDXUtility::CompileShader(
		L"Shaders\\PreFilteredSkyCubeMapPixelShader.sf",
		nullptr,
		"MainPS",
		"ps_5_1"
	);
}

void FPreFilteredSkyCubeMapRenderer::BuildPipelineState()
{
	// Create PipelineState Object
	D3D12_GRAPHICS_PIPELINE_STATE_DESC PreFilteredSkyCubeMapPipelineStateDesc;
	ZeroMemory(&PreFilteredSkyCubeMapPipelineStateDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
	auto mInputLayout = GetDrawingSphereInputLayouts();
	PreFilteredSkyCubeMapPipelineStateDesc.InputLayout = { mInputLayout.data(), (UINT)mInputLayout.size() };
	PreFilteredSkyCubeMapPipelineStateDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	PreFilteredSkyCubeMapPipelineStateDesc.SampleMask = UINT_MAX;
	PreFilteredSkyCubeMapPipelineStateDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	PreFilteredSkyCubeMapPipelineStateDesc.NumRenderTargets = 1;
	PreFilteredSkyCubeMapPipelineStateDesc.RTVFormats[0] = mCubeRenderTarget->GetFormat();
	PreFilteredSkyCubeMapPipelineStateDesc.SampleDesc.Count = 1;
	PreFilteredSkyCubeMapPipelineStateDesc.SampleDesc.Quality = 0;
	PreFilteredSkyCubeMapPipelineStateDesc.DSVFormat = mCubeRenderTarget->GetDepthStencilFormat();
	PreFilteredSkyCubeMapPipelineStateDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	PreFilteredSkyCubeMapPipelineStateDesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
	PreFilteredSkyCubeMapPipelineStateDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	PreFilteredSkyCubeMapPipelineStateDesc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
	PreFilteredSkyCubeMapPipelineStateDesc.pRootSignature = mRootSignature.Get();
	PreFilteredSkyCubeMapPipelineStateDesc.VS =
	{
		reinterpret_cast<BYTE*>(mVertexShader->GetBufferPointer()),
		mVertexShader->GetBufferSize()
	};
	PreFilteredSkyCubeMapPipelineStateDesc.PS =
	{
		reinterpret_cast<BYTE*>(mPixelShader->GetBufferPointer()),
		mPixelShader->GetBufferSize()
	};
	THROW_IF_FAILED(
		GetDXResourceManagerPtr()->GetDevicePtr()->CreateGraphicsPipelineState(
			&PreFilteredSkyCubeMapPipelineStateDesc,
			IID_PPV_ARGS(mPipelineState.GetAddressOf())
		)
	);
}

void FPreFilteredSkyCubeMapRenderer::UpdateCBs()
{
	// UpdateCB
	std::array<DirectX::XMFLOAT4X4, 6> CubeViews = FCubeRenderTarget::GetCubeMapViews();
	XMMATRIX P = XMMatrixPerspectiveFovLH(
		FDXMath::Pi / 2,
		mSkyTextureCube->Resource->GetDesc().Width / static_cast<float>(mSkyTextureCube->Resource->GetDesc().Height),
		0.1f,
		10.0f
	);
	UINT MipLevels = mCubeRenderTarget->GetMipLevels();
	for (int i = 0; i < 6; ++i)
	{
		FConstantBuffers CB;
		XMMATRIX V = XMLoadFloat4x4(&CubeViews[i]);
		XMMATRIX VP = XMMatrixMultiply(V, P);
		XMStoreFloat4x4(&CB.ViewProj, XMMatrixTranspose(VP));
		CB.SkyCubeMapIndex = mSkyTextureCube->SRVHeapIndex;
		CB.ResolutionOfSkyCubeMap = (float)mSkyTextureCube->Resource->GetDesc().Width;
		for (UINT j = 0; j < MipLevels; ++j)
		{
			CB.Roughness = j / max(1.0f, (MipLevels - 1.0f));
			mCB->CopyData(i * MipLevels + j, CB);
		}
	}
}

void FPreFilteredSkyCubeMapRenderer::Internal_Render(ID3D12Device* Device, ID3D12GraphicsCommandList* CommandList)
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

	CommandList->SetPipelineState(mPipelineState.Get());

	CommandList->SetGraphicsRootSignature(mRootSignature.Get());

	ID3D12DescriptorHeap* SRVHeap = GetTextureManager()->GetSRVHeapPtr();
	ID3D12DescriptorHeap* descriptorHeaps[] = { SRVHeap };
	CommandList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);
	CommandList->SetGraphicsRootDescriptorTable(1, GetTextureManager()->GetTextureCubeGPUSRVForHeapStart());

	UINT MipLevels = mCubeRenderTarget->GetMipLevels();
	for (int i = 0; i < 6; ++i)
	{
		for (UINT j = 0; j < MipLevels; ++j)
		{
			D3D12_VIEWPORT Viewport = mCubeRenderTarget->GetViewportMipLevel(j);
			D3D12_RECT ScissorRect = mCubeRenderTarget->GetScissorRectMipLevel(j);
			CommandList->RSSetViewports(1, &Viewport);
			CommandList->RSSetScissorRects(1, &ScissorRect);

			D3D12_CPU_DESCRIPTOR_HANDLE RTV = mCubeRenderTarget->GetRTV(i, j);
			D3D12_CPU_DESCRIPTOR_HANDLE DSV = mCubeRenderTarget->GetDSVHeap()->GetCPUDescriptorHandleForHeapStart();
			CommandList->ClearRenderTargetView(
				RTV,
				DirectX::Colors::Black,
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
			ID3D12Resource* CB = mCB->Resource();
			UINT CBSize = FDXUtility::CalcConstantBufferByteSize(sizeof(FConstantBuffers));
			D3D12_GPU_VIRTUAL_ADDRESS CBAddress = CB->GetGPUVirtualAddress() + ((i * MipLevels + j) * CBSize);
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
	CreatePreFilteredSkyCubeMap();
	CreateIndirectSpecularIntegral();
}

void FCubeSkyRenderer::Render(ID3D12GraphicsCommandList* CommandList)
{
	UpdateCBs();
	CommandList->SetPipelineState(mPipelineState.Get());
	CommandList->SetGraphicsRootSignature(mRootSignature.Get());
	CommandList->SetGraphicsRootConstantBufferView(0, mCB->Resource()->GetGPUVirtualAddress());
	CommandList->SetGraphicsRootDescriptorTable(1, GetTextureManager()->GetTextureCubeGPUSRVForHeapStart());

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
	D3D12_DESCRIPTOR_RANGE CubeTextureTable = GetTextureManager()->GetTextureCubeDescriptorRange();

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

void FCubeSkyRenderer::CreatePreFilteredSkyCubeMap()
{
	mPreFilteredSkyCubeMapName = mSkyTextureCube->Name + "_Specular";
	mPreFilteredSkyCubeMapRenderTarget = std::make_unique<FCubeRenderTarget>(
		mPreFilteredSkyCubeMapName,
		(UINT)mSkyTextureCube->Resource->GetDesc().Width,
		(UINT)mSkyTextureCube->Resource->GetDesc().Height,
		5u
	);
	mPreFilteredSkyCubeMapSRVHeapIndex = mPreFilteredSkyCubeMapRenderTarget->mTexture->SRVHeapIndex;
	FPreFilteredSkyCubeMapRenderer PreFilteredSkyCubeMapRenderer(mPreFilteredSkyCubeMapRenderTarget.get());
	PreFilteredSkyCubeMapRenderer.Render(mSkyTextureCube);
}

void FCubeSkyRenderer::CreateIndirectSpecularIntegral()
{
	mIndirectSpecularIntegralTextureName = "IndirectSpecularIntegral";
	mIndirectSpecularIntegralRenderTarget = std::make_unique<FRenderTarget>(
		mIndirectSpecularIntegralTextureName,
		512u,
		512u
	);
	mIndirectSpecularIntegralTextureSRVHeapIndex = mIndirectSpecularIntegralRenderTarget->GetTexture()->SRVHeapIndex;
	FIndirectSpecularIntegralRenderer Renderer(mIndirectSpecularIntegralRenderTarget.get());
	Renderer.Render(mSkyTextureCube);
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
	CB.SkyCubeMapIndex = mSkyTextureCube->SRVHeapIndex;
	mCB->CopyData(0, CB);
}

FIndirectSpecularIntegralRenderer::FIndirectSpecularIntegralRenderer(FRenderTarget* RenderTarget):
	mRenderTarget(RenderTarget)
{
	BuildCB();
	BuildRootSignature();
	BuildShaders();
	BuildPipelineState();
}

void FIndirectSpecularIntegralRenderer::Render(FTexture* SkyTextureCube)
{
	mSkyTextureCube = SkyTextureCube;
	UpdateCBs();
	GetDXResourceManagerPtr()->ExecuteAndFlushCommand(&FIndirectSpecularIntegralRenderer::Internal_Render, this);
}

void FIndirectSpecularIntegralRenderer::BuildCB()
{
	// Build CB
	mCB = std::make_unique <TUploadBuffer<FConstantBuffers>>(
		GetDXResourceManagerPtr()->GetDevicePtr(),
		1,
		true
	);
}

void FIndirectSpecularIntegralRenderer::BuildRootSignature()
{
	// Build RootSignature
	D3D12_DESCRIPTOR_RANGE CubeTextureTable = GetTextureManager()->GetTextureCubeDescriptorRange();

	// Tip: 자주 사용되는 것일수록 작은 인덱스에 보관하는게 퍼포먼스가 좋음
	constexpr UINT ROOT_PARAMETERs_NUM = 1;
	CD3DX12_ROOT_PARAMETER RootParameter[ROOT_PARAMETERs_NUM];
	RootParameter[0].InitAsDescriptorTable(1, &CubeTextureTable, D3D12_SHADER_VISIBILITY_PIXEL);	// CubeTextureTable

	FDXUtility::BuildRootSignature(
		RootParameter,
		ROOT_PARAMETERs_NUM,
		mRootSignature.GetAddressOf()
	);
}

void FIndirectSpecularIntegralRenderer::BuildShaders()
{
	// Compile Shader
	mVertexShader = FDXUtility::CompileShader(
		L"Shaders\\IndirectSpecularIntegralVertexShader.sf",
		nullptr,
		"MainVS",
		"vs_5_1"
	);

	mPixelShader = FDXUtility::CompileShader(
		L"Shaders\\IndirectSpecularIntegralPixelShader.sf",
		nullptr,
		"MainPS",
		"ps_5_1"
	);
}

void FIndirectSpecularIntegralRenderer::BuildPipelineState()
{
	// Create PipelineState Object
	D3D12_GRAPHICS_PIPELINE_STATE_DESC IndirectSpecularIntegralPipelineStateDesc;
	ZeroMemory(&IndirectSpecularIntegralPipelineStateDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
	auto mInputLayout = GetDrawingRectInputLayouts();
	IndirectSpecularIntegralPipelineStateDesc.InputLayout = { mInputLayout.data(), (UINT)mInputLayout.size() };
	IndirectSpecularIntegralPipelineStateDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	IndirectSpecularIntegralPipelineStateDesc.SampleMask = UINT_MAX;
	IndirectSpecularIntegralPipelineStateDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	IndirectSpecularIntegralPipelineStateDesc.NumRenderTargets = 1;
	IndirectSpecularIntegralPipelineStateDesc.RTVFormats[0] = mRenderTarget->GetFormat();
	IndirectSpecularIntegralPipelineStateDesc.SampleDesc.Count = 1;
	IndirectSpecularIntegralPipelineStateDesc.SampleDesc.Quality = 0;
	IndirectSpecularIntegralPipelineStateDesc.DSVFormat = mRenderTarget->GetDepthStencilFormat();
	IndirectSpecularIntegralPipelineStateDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	IndirectSpecularIntegralPipelineStateDesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
	IndirectSpecularIntegralPipelineStateDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	IndirectSpecularIntegralPipelineStateDesc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
	IndirectSpecularIntegralPipelineStateDesc.pRootSignature = mRootSignature.Get();
	IndirectSpecularIntegralPipelineStateDesc.VS =
	{
		reinterpret_cast<BYTE*>(mVertexShader->GetBufferPointer()),
		mVertexShader->GetBufferSize()
	};
	IndirectSpecularIntegralPipelineStateDesc.PS =
	{
		reinterpret_cast<BYTE*>(mPixelShader->GetBufferPointer()),
		mPixelShader->GetBufferSize()
	};
	THROW_IF_FAILED(
		GetDXResourceManagerPtr()->GetDevicePtr()->CreateGraphicsPipelineState(
			&IndirectSpecularIntegralPipelineStateDesc,
			IID_PPV_ARGS(mPipelineState.GetAddressOf())
		)
	);
}

void FIndirectSpecularIntegralRenderer::UpdateCBs()
{
	// UpdateCB
	//FConstantBuffers CB;
	//XMMATRIX V = XMLoadFloat4x4(&CubeViews[i]);
	//XMMATRIX VP = XMMatrixMultiply(V, P);
	//XMStoreFloat4x4(&CB.ViewProj, XMMatrixTranspose(VP));
	//CB.SkyCubeMapIndex = mSkyTextureCube->SRVHeapIndex;
	//CB.ResolutionOfSkyCubeMap = (float)mSkyTextureCube->Resource->GetDesc().Width;
	//for (UINT j = 0; j < MipLevels; ++j)
	//{
	//	CB.Roughness = j / max(1.0f, (MipLevels - 1.0f));
	//	mCB->CopyData(i * MipLevels + j, CB);
	//}
}

void FIndirectSpecularIntegralRenderer::Internal_Render(ID3D12Device* Device, ID3D12GraphicsCommandList* CommandList)
{
	D3D12_RESOURCE_BARRIER ResourceBarrier = CD3DX12_RESOURCE_BARRIER::Transition(
		mRenderTarget->GetDepthStencilResource(),
		D3D12_RESOURCE_STATE_COMMON,
		D3D12_RESOURCE_STATE_DEPTH_WRITE
	);
	CommandList->ResourceBarrier(
		1,
		&ResourceBarrier
	);

	ResourceBarrier = CD3DX12_RESOURCE_BARRIER::Transition(
		mRenderTarget->GetResource(),
		D3D12_RESOURCE_STATE_GENERIC_READ,
		D3D12_RESOURCE_STATE_RENDER_TARGET
	);
	CommandList->ResourceBarrier(
		1,
		&ResourceBarrier
	);

	CommandList->SetPipelineState(mPipelineState.Get());

	CommandList->SetGraphicsRootSignature(mRootSignature.Get());

	D3D12_VIEWPORT Viewport = mRenderTarget->GetViewport();
	D3D12_RECT ScissorRect = mRenderTarget->GetScissorRect();
	CommandList->RSSetViewports(1, &Viewport);
	CommandList->RSSetScissorRects(1, &ScissorRect);

	D3D12_CPU_DESCRIPTOR_HANDLE RTV = mRenderTarget->GetRTV(0);
	D3D12_CPU_DESCRIPTOR_HANDLE DSV = mRenderTarget->GetDSVHeap()->GetCPUDescriptorHandleForHeapStart();
	CommandList->OMSetRenderTargets(1, &RTV, true, &DSV);
	CommandList->ClearRenderTargetView(
		RTV,
		DirectX::Colors::Black,
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
	DrawRect(CommandList);
	

	ResourceBarrier = CD3DX12_RESOURCE_BARRIER::Transition(
		mRenderTarget->GetResource(),
		D3D12_RESOURCE_STATE_RENDER_TARGET,
		D3D12_RESOURCE_STATE_GENERIC_READ
	);
	CommandList->ResourceBarrier(
		1,
		&ResourceBarrier
	);


	ResourceBarrier = CD3DX12_RESOURCE_BARRIER::Transition(
		mRenderTarget->GetDepthStencilResource(),
		D3D12_RESOURCE_STATE_DEPTH_WRITE,
		D3D12_RESOURCE_STATE_COMMON
	);
	CommandList->ResourceBarrier(
		1,
		&ResourceBarrier
	);
}
