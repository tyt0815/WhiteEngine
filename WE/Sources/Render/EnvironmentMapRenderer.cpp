#include "EnvironmentMapRenderer.h"
#include "DirectX/DXResourceManager.h"
#include "DirectX/DXMath.h"
#include "DirectX/CBVSRVUAVHeap.h"
#include "ShapeDrawer.h"
#include "GameFramework/Object/World/World.h"
#include "GameFramework/Object/Component/CameraComponent.h"

FEnvironmentMapRenderer::FEnvironmentMapRenderer(FTexture* TextureCube):
	mDevice(GetDXResourceManagerPtr()->GetDevicePtr()),
	mSkyTextureCube(TextureCube)
{
	BuildRenderTargetsAndDepthStencil();
	BuildBuffers();
	BuildRootSignatures();
	BuildShadersAndInputLayouts();
	BuildPipelineStates();
	GetDXResourceManagerPtr()->ExecuteAndFlushCommand(&FEnvironmentMapRenderer::PreRender, this);
}

void FEnvironmentMapRenderer::Render(
	ID3D12GraphicsCommandList* CommandList,
	D3D12_CPU_DESCRIPTOR_HANDLE Rtv,
	D3D12_CPU_DESCRIPTOR_HANDLE Dsv,
	D3D12_VIEWPORT Viewport
)
{
	UpdateBuffers();

	CommandList->RSSetViewports(1, &Viewport);
	D3D12_RECT ScissorRect = FDXUtility::MakeScissorRectFromViewport(Viewport);
	CommandList->RSSetScissorRects(1, &ScissorRect);
	CommandList->OMSetRenderTargets(1, &Rtv, false, &Dsv);

	CommandList->SetPipelineState(mPipelineStates["EnvironmentMapPass"].Get());

	CommandList->SetGraphicsRootSignature(mRootSignatures["EnvironmentMapPass"].Get());
	FCBVSRVUAVHeap* SRVHeap = GetCBVSRVUAVHeap();
	ID3D12DescriptorHeap* descriptorHeaps[] = { SRVHeap->Get() };
	CommandList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);
	CommandList->SetGraphicsRootConstantBufferView(0, mEnvironmentMapPassCB->Resource()->GetGPUVirtualAddress());
	CommandList->SetGraphicsRootDescriptorTable(1, SRVHeap->GetTextureCubeGPUDescriptorHandleStart());

	DrawSphere(CommandList);
}

void FEnvironmentMapRenderer::BuildRenderTargetsAndDepthStencil()
{
	mIrradianceMapRenderTarget = std::make_unique<FCubeRenderTarget>(
		(UINT)mSkyTextureCube->GetWidth(),
		(UINT)mSkyTextureCube->GetHeight()
		);
	mPrefilteredMapRenderTarget = std::make_unique<FCubeRenderTarget>(
		(UINT)mSkyTextureCube->GetWidth(),
		(UINT)mSkyTextureCube->GetHeight(),
		5u
	);
	mBRDFLUTRenderTarget = std::make_unique<FRenderTarget>(512u, 512u);

	mDepthStencil = std::make_unique<FDepthStencil>(
		(UINT)max(mSkyTextureCube->GetWidth(), mBRDFLUTRenderTarget->GetWidth()),
		(UINT)max(mSkyTextureCube->GetHeight(), mBRDFLUTRenderTarget->GetHeight())
	);
}

void FEnvironmentMapRenderer::BuildBuffers()
{
	mIrradianceMapPassCB = std::make_unique <TUploadBuffer<FIrradianceMapPassCB>>(mDevice, 6, true);
	mPreFilteredMapPassCB = std::make_unique <TUploadBuffer<FPreFilteredMapPassCB>>(
		mDevice,
		6 * mPrefilteredMapRenderTarget->GetMipLevels(),
		true);
	mEnvironmentMapPassCB = std::make_unique <TUploadBuffer<FEnvironmentMapPassCB>>(mDevice, 1, true);
}

void FEnvironmentMapRenderer::BuildRootSignatures()
{
	BuildIrradianceMapPassRootSignature();
	BuildPreFilteredMapPassRootSignature();
	BuildBRDFLUTPassRootSignature();
	BuildEnvironmentMapPassRootSignature();
}

void FEnvironmentMapRenderer::BuildIrradianceMapPassRootSignature()
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
		mRootSignatures["IrradianceMapPass"].GetAddressOf()
	);
}

void FEnvironmentMapRenderer::BuildPreFilteredMapPassRootSignature()
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
		mRootSignatures["PreFilteredMapPass"].GetAddressOf()
	);
}

void FEnvironmentMapRenderer::BuildBRDFLUTPassRootSignature()
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
		mRootSignatures["BRDFLUTPass"].GetAddressOf()
	);
}

void FEnvironmentMapRenderer::BuildEnvironmentMapPassRootSignature()
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
		mRootSignatures["EnvironmentMapPass"].GetAddressOf()
	);
}

void FEnvironmentMapRenderer::BuildShadersAndInputLayouts()
{
	BuildIrradianceMapPassShaders();
	BuildPreFilteredMapPassShaders();
	BuildBRDFLUTPassShaders();
	BuildEnvironmentMapPassShaders();
}

void FEnvironmentMapRenderer::BuildIrradianceMapPassShaders()
{
	mShaders["IrradianceMapPassVertexShader"] = FDXUtility::CompileShader(
		L"Shaders\\IrradianceMapPassVertexShader.hlsl",
		nullptr,
		"MainVS",
		"vs_5_1"
	);

	mShaders["IrradianceMapPassPixelShader"] = FDXUtility::CompileShader(
		L"Shaders\\IrradianceMapPassPixelShader.hlsl",
		nullptr,
		"MainPS",
		"ps_5_1"
	);
}

void FEnvironmentMapRenderer::BuildPreFilteredMapPassShaders()
{
	mShaders["PreFilteredMapPassVertexShader"] = FDXUtility::CompileShader(
		L"Shaders\\PreFilteredMapPassVertexShader.hlsl",
		nullptr,
		"MainVS",
		"vs_5_1"
	);

	mShaders["PreFilteredMapPassPixelShader"] = FDXUtility::CompileShader(
		L"Shaders\\PreFilteredMapPassPixelShader.hlsl",
		nullptr,
		"MainPS",
		"ps_5_1"
	);
}

void FEnvironmentMapRenderer::BuildBRDFLUTPassShaders()
{
	// Compile Shader
	mShaders["BRDFLUTPassVertexShader"] = FDXUtility::CompileShader(
		L"Shaders\\BRDFLUTPassVertexShader.hlsl",
		nullptr,
		"MainVS",
		"vs_5_1"
	);

	mShaders["BRDFLUTPassPixelShader"] = FDXUtility::CompileShader(
		L"Shaders\\BRDFLUTPassPixelShader.hlsl",
		nullptr,
		"MainPS",
		"ps_5_1"
	);
}

void FEnvironmentMapRenderer::BuildEnvironmentMapPassShaders()
{
	mShaders["EnvironmentMapPassVertexShader"] = FDXUtility::CompileShader(
		L"Shaders\\EnvironmentMapPassVertexShader.hlsl",
		nullptr,
		"MainVS",
		"vs_5_1"
	);

	mShaders["EnvironmentMapPassPixelShader"] = FDXUtility::CompileShader(
		L"Shaders\\EnvironmentMapPassPixelShader.hlsl",
		nullptr,
		"MainPS",
		"ps_5_1"
	);
}

void FEnvironmentMapRenderer::BuildPipelineStates()
{
	BuildIrradianceMapPassPipelineState();
	BuildPreFilteredMapPassPipelineState();
	BuildBRDFLUTPassPipelineState();
	BuildEnvironmentMapPassPipelineState();
}

void FEnvironmentMapRenderer::BuildIrradianceMapPassPipelineState()
{
	// Create PipelineState Object
	D3D12_GRAPHICS_PIPELINE_STATE_DESC Desc;
	ZeroMemory(&Desc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
	auto mInputLayout = GetDrawingSphereInputLayouts();
	Desc.InputLayout = { mInputLayout.data(), (UINT)mInputLayout.size() };
	Desc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	Desc.SampleMask = UINT_MAX;
	Desc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	Desc.NumRenderTargets = 1;
	Desc.RTVFormats[0] = mIrradianceMapRenderTarget->GetFormat();
	Desc.SampleDesc.Count = 1;
	Desc.SampleDesc.Quality = 0;
	Desc.DSVFormat = mDepthStencil->GetFormat();
	Desc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	Desc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
	Desc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	Desc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
	Desc.pRootSignature = mRootSignatures["IrradianceMapPass"].Get();
	Desc.VS =
	{
		reinterpret_cast<BYTE*>(mShaders["IrradianceMapPassVertexShader"]->GetBufferPointer()),
		mShaders["IrradianceMapPassVertexShader"]->GetBufferSize()
	};
	Desc.PS =
	{
		reinterpret_cast<BYTE*>(mShaders["IrradianceMapPassPixelShader"]->GetBufferPointer()),
		mShaders["IrradianceMapPassPixelShader"]->GetBufferSize()
	};
	THROW_IF_FAILED(
		mDevice->CreateGraphicsPipelineState(
			&Desc,
			IID_PPV_ARGS(mPipelineStates["IrradianceMapPass"].GetAddressOf())
		)
	);
}

void FEnvironmentMapRenderer::BuildPreFilteredMapPassPipelineState()
{
	// Create PipelineState Object
	D3D12_GRAPHICS_PIPELINE_STATE_DESC Desc;
	ZeroMemory(&Desc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
	auto mInputLayout = GetDrawingSphereInputLayouts();
	Desc.InputLayout = { mInputLayout.data(), (UINT)mInputLayout.size() };
	Desc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	Desc.SampleMask = UINT_MAX;
	Desc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	Desc.NumRenderTargets = 1;
	Desc.RTVFormats[0] = mPrefilteredMapRenderTarget->GetFormat();
	Desc.SampleDesc.Count = 1;
	Desc.SampleDesc.Quality = 0;
	Desc.DSVFormat = mDepthStencil->GetFormat();
	Desc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	Desc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
	Desc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	Desc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
	Desc.pRootSignature = mRootSignatures["PreFilteredMapPass"].Get();
	Desc.VS =
	{
		reinterpret_cast<BYTE*>(mShaders["PreFilteredMapPassVertexShader"]->GetBufferPointer()),
		mShaders["PreFilteredMapPassVertexShader"]->GetBufferSize()
	};
	Desc.PS =
	{
		reinterpret_cast<BYTE*>(mShaders["PreFilteredMapPassPixelShader"]->GetBufferPointer()),
		mShaders["PreFilteredMapPassPixelShader"]->GetBufferSize()
	};
	THROW_IF_FAILED(
		mDevice->CreateGraphicsPipelineState(
			&Desc,
			IID_PPV_ARGS(mPipelineStates["PreFilteredMapPass"].GetAddressOf())
		)
	);
}

void FEnvironmentMapRenderer::BuildBRDFLUTPassPipelineState()
{
	// Create PipelineState Object
	D3D12_GRAPHICS_PIPELINE_STATE_DESC Desc;
	ZeroMemory(&Desc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
	auto mInputLayout = GetDrawingRectInputLayouts();
	Desc.InputLayout = { mInputLayout.data(), (UINT)mInputLayout.size() };
	Desc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	Desc.SampleMask = UINT_MAX;
	Desc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	Desc.NumRenderTargets = 1;
	Desc.RTVFormats[0] = mBRDFLUTRenderTarget->GetFormat();
	Desc.SampleDesc.Count = 1;
	Desc.SampleDesc.Quality = 0;
	Desc.DSVFormat = mDepthStencil->GetFormat();
	Desc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	Desc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
	Desc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	Desc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
	Desc.pRootSignature = mRootSignatures["BRDFLUTPass"].Get();
	Desc.VS =
	{
		reinterpret_cast<BYTE*>(mShaders["BRDFLUTPassVertexShader"]->GetBufferPointer()),
		mShaders["BRDFLUTPassVertexShader"]->GetBufferSize()
	};
	Desc.PS =
	{
		reinterpret_cast<BYTE*>(mShaders["BRDFLUTPassPixelShader"]->GetBufferPointer()),
		mShaders["BRDFLUTPassPixelShader"]->GetBufferSize()
	};
	THROW_IF_FAILED(
		mDevice->CreateGraphicsPipelineState(
			&Desc,
			IID_PPV_ARGS(mPipelineStates["BRDFLUTPass"].GetAddressOf())
		)
	);
}

void FEnvironmentMapRenderer::BuildEnvironmentMapPassPipelineState()
{
	D3D12_GRAPHICS_PIPELINE_STATE_DESC Desc;
	ZeroMemory(&Desc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
	auto InputLayout = GetDrawingSphereInputLayouts();
	Desc.InputLayout = { InputLayout.data(), (UINT)InputLayout.size() };
	Desc.pRootSignature = mRootSignatures["EnvironmentMapPass"].Get();
	Desc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	Desc.SampleMask = UINT_MAX;
	Desc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	Desc.NumRenderTargets = 1;
	Desc.RTVFormats[0] = GetDXResourceManagerPtr()->GetBackbufferFormat();
	Desc.SampleDesc.Count = GetDXResourceManagerPtr()->IsMSAAOn() ? 4 : 1;
	Desc.SampleDesc.Quality = GetDXResourceManagerPtr()->IsMSAAOn() ? (GetDXResourceManagerPtr()->GetMSAAQuality_4x() - 1) : 0;
	Desc.DSVFormat = GetDXResourceManagerPtr()->GetDepthStencilFormat();
	Desc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	Desc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
	Desc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	Desc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
	Desc.VS =
	{
		reinterpret_cast<BYTE*>(mShaders["EnvironmentMapPassVertexShader"]->GetBufferPointer()),
		mShaders["EnvironmentMapPassVertexShader"]->GetBufferSize()
	};
	Desc.PS =
	{
		reinterpret_cast<BYTE*>(mShaders["EnvironmentMapPassPixelShader"]->GetBufferPointer()),
		mShaders["EnvironmentMapPassPixelShader"]->GetBufferSize()
	};
	THROW_IF_FAILED(
		mDevice->CreateGraphicsPipelineState(
			&Desc,
			IID_PPV_ARGS(mPipelineStates["EnvironmentMapPass"].GetAddressOf())
		)
	);
}

void FEnvironmentMapRenderer::PreRender(ID3D12Device* Device, ID3D12GraphicsCommandList* CommandList)
{
	PreRenderIrradianceMapPass(CommandList);
	PreRenderPrefilteredMapPass(CommandList);
	PreRenderBRDFLUTPass(CommandList);
}

void FEnvironmentMapRenderer::PreRenderIrradianceMapPass(ID3D12GraphicsCommandList* CommandList)
{
	// UpdateCBs
	std::array<DirectX::XMFLOAT4X4, 6> CubeViews = FCubeRenderTarget::GetCubeMapViews();
	DirectX::XMMATRIX P = DirectX::XMMatrixPerspectiveFovLH(FDXMath::Pi / 2, 1.0f, 0.1f, 10.0f);
	for (int i = 0; i < 6; ++i)
	{
		FIrradianceMapPassCB CB;
		XMMATRIX V = XMLoadFloat4x4(&CubeViews[i]);
		XMMATRIX VP = XMMatrixMultiply(V, P);
		XMStoreFloat4x4(&CB.ViewProj, XMMatrixTranspose(VP));
		CB.SkyCubeMapIndex = mSkyTextureCube->GetSRVHeapIndex();
		mIrradianceMapPassCB->CopyData(i, CB);
	}

	// Render
	mIrradianceMapRenderTarget->TransitionResourceBarrier(CommandList, D3D12_RESOURCE_STATE_RENDER_TARGET);
	mDepthStencil->TransitionResourceBarrier(CommandList, D3D12_RESOURCE_STATE_DEPTH_WRITE);

	D3D12_VIEWPORT Viewport = mIrradianceMapRenderTarget->GetViewport();
	D3D12_RECT ScissorRect = mIrradianceMapRenderTarget->GetScissorRect();
	CommandList->RSSetViewports(1, &Viewport);
	CommandList->RSSetScissorRects(1, &ScissorRect);

	CommandList->SetPipelineState(mPipelineStates["IrradianceMapPass"].Get());

	CommandList->SetGraphicsRootSignature(mRootSignatures["IrradianceMapPass"].Get());

	FCBVSRVUAVHeap* SRVHeap = GetCBVSRVUAVHeap();
	ID3D12DescriptorHeap* descriptorHeaps[] = { SRVHeap->Get()};
	CommandList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);
	CommandList->SetGraphicsRootDescriptorTable(1, SRVHeap->GetTextureCubeGPUDescriptorHandleStart());
	for (int i = 0; i < 6; ++i)
	{
		mIrradianceMapRenderTarget->Clear(CommandList, i, 0);
		mDepthStencil->Clear(CommandList);

		D3D12_CPU_DESCRIPTOR_HANDLE RTV = mIrradianceMapRenderTarget->GetRTV(i, 0);
		D3D12_CPU_DESCRIPTOR_HANDLE DSV = mDepthStencil->GetDSV();
		CommandList->OMSetRenderTargets(1, &RTV, true, &DSV);

		ID3D12Resource* CB = mIrradianceMapPassCB->Resource();
		UINT CBSize = FDXUtility::CalcConstantBufferByteSize(sizeof(FIrradianceMapPassCB));
		D3D12_GPU_VIRTUAL_ADDRESS CBAddress = CB->GetGPUVirtualAddress() + (i * CBSize);
		CommandList->SetGraphicsRootConstantBufferView(0, CBAddress);
		DrawSphere(CommandList);
	}

	mIrradianceMapRenderTarget->TransitionResourceBarrier(CommandList, D3D12_RESOURCE_STATE_GENERIC_READ);
	mDepthStencil->TransitionResourceBarrier(CommandList, D3D12_RESOURCE_STATE_DEPTH_READ);
}

void FEnvironmentMapRenderer::PreRenderPrefilteredMapPass(ID3D12GraphicsCommandList* CommandList)
{
	// UpdateCB
	std::array<DirectX::XMFLOAT4X4, 6> CubeViews = FCubeRenderTarget::GetCubeMapViews();
	XMMATRIX P = XMMatrixPerspectiveFovLH(
		FDXMath::Pi / 2,
		mSkyTextureCube->GetWidth() / static_cast<float>(mSkyTextureCube->GetHeight()),
		0.1f,
		10.0f
	);
	UINT MipLevels = mPrefilteredMapRenderTarget->GetMipLevels();
	for (int i = 0; i < 6; ++i)
	{
		FPreFilteredMapPassCB CB;
		XMMATRIX V = XMLoadFloat4x4(&CubeViews[i]);
		XMMATRIX VP = XMMatrixMultiply(V, P);
		XMStoreFloat4x4(&CB.ViewProj, XMMatrixTranspose(VP));
		CB.SkyCubeMapIndex = mSkyTextureCube->GetSRVHeapIndex();
		CB.ResolutionOfSkyCubeMap = (float)mSkyTextureCube->GetWidth();
		for (UINT j = 0; j < MipLevels; ++j)
		{
			CB.Roughness = j / max(1.0f, (MipLevels - 1.0f));
			mPreFilteredMapPassCB->CopyData(i * MipLevels + j, CB);
		}
	}

	//////////////////////////////////////////////////////////////////////////
	// Render
	mPrefilteredMapRenderTarget->TransitionResourceBarrier(CommandList, D3D12_RESOURCE_STATE_RENDER_TARGET);
	mDepthStencil->TransitionResourceBarrier(CommandList, D3D12_RESOURCE_STATE_DEPTH_WRITE);

	CommandList->SetPipelineState(mPipelineStates["PreFilteredMapPass"].Get());

	CommandList->SetGraphicsRootSignature(mRootSignatures["PreFilteredMapPass"].Get());

	FCBVSRVUAVHeap* SRVHeap = GetCBVSRVUAVHeap();
	ID3D12DescriptorHeap* descriptorHeaps[] = { SRVHeap->Get()};
	CommandList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);
	CommandList->SetGraphicsRootDescriptorTable(1, SRVHeap->GetTextureCubeGPUDescriptorHandleStart());

	for (int i = 0; i < 6; ++i)
	{
		for (UINT j = 0; j < MipLevels; ++j)
		{
			D3D12_VIEWPORT Viewport = mPrefilteredMapRenderTarget->GetViewportMipLevel(j);
			D3D12_RECT ScissorRect = mPrefilteredMapRenderTarget->GetScissorRectMipLevel(j);
			CommandList->RSSetViewports(1, &Viewport);
			CommandList->RSSetScissorRects(1, &ScissorRect);

			mPrefilteredMapRenderTarget->Clear(CommandList, i, j);
			mDepthStencil->Clear(CommandList);

			D3D12_CPU_DESCRIPTOR_HANDLE RTV = mPrefilteredMapRenderTarget->GetRTV(i, j);
			D3D12_CPU_DESCRIPTOR_HANDLE DSV = mDepthStencil->GetDSV();

			CommandList->OMSetRenderTargets(1, &RTV, true, &DSV);
			ID3D12Resource* CB = mPreFilteredMapPassCB->Resource();
			UINT CBSize = FDXUtility::CalcConstantBufferByteSize(sizeof(FPreFilteredMapPassCB));
			D3D12_GPU_VIRTUAL_ADDRESS CBAddress = CB->GetGPUVirtualAddress() + ((i * MipLevels + j) * CBSize);
			CommandList->SetGraphicsRootConstantBufferView(0, CBAddress);

			DrawSphere(CommandList);
		}
	}

	mPrefilteredMapRenderTarget->TransitionResourceBarrier(CommandList, D3D12_RESOURCE_STATE_GENERIC_READ);
	mDepthStencil->TransitionResourceBarrier(CommandList, D3D12_RESOURCE_STATE_DEPTH_READ);
}

void FEnvironmentMapRenderer::PreRenderBRDFLUTPass(ID3D12GraphicsCommandList* CommandList)
{
	mBRDFLUTRenderTarget->TransitionResourceBarrier(CommandList, D3D12_RESOURCE_STATE_RENDER_TARGET);
	mDepthStencil->TransitionResourceBarrier(CommandList, D3D12_RESOURCE_STATE_DEPTH_WRITE);

	D3D12_VIEWPORT Viewport = mBRDFLUTRenderTarget->GetViewport();
	D3D12_RECT ScissorRect = mBRDFLUTRenderTarget->GetScissorRect();
	CommandList->RSSetViewports(1, &Viewport);
	CommandList->RSSetScissorRects(1, &ScissorRect);

	mBRDFLUTRenderTarget->Clear(CommandList);
	mDepthStencil->Clear(CommandList);

	D3D12_CPU_DESCRIPTOR_HANDLE RTV = mBRDFLUTRenderTarget->GetRTV(0);
	D3D12_CPU_DESCRIPTOR_HANDLE DSV = mDepthStencil->GetDSV();
	CommandList->OMSetRenderTargets(1, &RTV, false, &DSV);
	CommandList->SetPipelineState(mPipelineStates["BRDFLUTPass"].Get());
	CommandList->SetGraphicsRootSignature(mRootSignatures["BRDFLUTPass"].Get());
	DrawRect(CommandList);


	mBRDFLUTRenderTarget->TransitionResourceBarrier(CommandList, D3D12_RESOURCE_STATE_GENERIC_READ);
	mDepthStencil->TransitionResourceBarrier(CommandList, D3D12_RESOURCE_STATE_DEPTH_READ);
}

void FEnvironmentMapRenderer::UpdateBuffers()
{
	WCameraComponent* Camera = GetWorld()->GetPlayerCamera();
	FEnvironmentMapPassCB CB;
	CB.EyePosW = Camera->GetLocalLocation();
	XMFLOAT4X4 View = Camera->GetViewMatrix();
	XMFLOAT4X4 Proj = Camera->GetProjMatrix();
	XMMATRIX V = XMLoadFloat4x4(&View);
	XMMATRIX P = XMLoadFloat4x4(&Proj);
	XMMATRIX VP = V * P;
	XMStoreFloat4x4(&CB.ViewProj, XMMatrixTranspose(VP));
	CB.SkyCubeMapIndex = mSkyTextureCube->GetSRVHeapIndex();
	mEnvironmentMapPassCB->CopyData(0, CB);
}
