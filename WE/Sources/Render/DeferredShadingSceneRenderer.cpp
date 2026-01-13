
#include "DeferredShadingSceneRenderer.h"
#include "DirectXColors.h"
#include "DirectX/DXResourceManager.h"
#include "DirectX/CBVSRVUAVHeap.h"
#include "GameFramework/InputSystem/InputSystem.h"
#include "ShapeDrawer.h"

#include "SkeletalMesh.h"
#include "MeshGeometry.h"

FDeferredShadingSceneRenderer::FFrameResource::FFrameResource(ID3D12Device* Device) :
	FFrameResourceBase(Device)
{
	mDeferredShadingPassCB = std::make_unique<TUploadBuffer<FDeferredShadingPassConstantBuffer>>(Device, 1, true);
}

FDeferredShadingSceneRenderer::FDeferredShadingSceneRenderer()
{
	mGBufferA = std::make_unique<FRenderTarget>(
		(UINT)GetDXResourceManagerPtr()->GetScreenViewport().Width,
		(UINT)GetDXResourceManagerPtr()->GetScreenViewport().Height,
		1,
		DXGI_FORMAT_R16G16B16A16_FLOAT
	);
	mGBufferB = std::make_unique<FRenderTarget>(
		(UINT)GetDXResourceManagerPtr()->GetScreenViewport().Width,
		(UINT)GetDXResourceManagerPtr()->GetScreenViewport().Height
	);
	mGBufferC = std::make_unique<FRenderTarget>(
		(UINT)GetDXResourceManagerPtr()->GetScreenViewport().Width,
		(UINT)GetDXResourceManagerPtr()->GetScreenViewport().Height
	);
	mGBufferDepthStencil = std::make_unique<FDepthStencil>(
		(UINT)GetDXResourceManagerPtr()->GetScreenViewport().Width,
		(UINT)GetDXResourceManagerPtr()->GetScreenViewport().Height
	);

	mEnvironmentMapRenderer = std::make_unique<FEnvironmentMapRenderer>(GetTextureManager()->GetTexture("SnowCube"));
}

void FDeferredShadingSceneRenderer::Initialize(ID3D12Device* Device)
{
	Super::Initialize(Device);
	
	for (int i = 0; i < mFrameResources.size(); ++i)
	{
		FFrameResource* FrameResource = dynamic_cast<FFrameResource*>(mFrameResources[i].get());
		UpdateDeferredShadingPassCB(FrameResource->GetDeferredShadingPassCB());
	}

	GetInputSystemManager()->BindKeyboardAction('1', this, &FDeferredShadingSceneRenderer::SwitchToDefaultMode);
	GetInputSystemManager()->BindKeyboardAction('2', this, &FDeferredShadingSceneRenderer::SwitchToBlurMode);
	GetInputSystemManager()->BindKeyboardAction('3', this, &FDeferredShadingSceneRenderer::SwitchToGBufferADebugMode);
	GetInputSystemManager()->BindKeyboardAction('4', this, &FDeferredShadingSceneRenderer::SwitchToGBufferBDebugMode);
	GetInputSystemManager()->BindKeyboardAction('5', this, &FDeferredShadingSceneRenderer::SwitchToGBufferCDebugMode);
	GetInputSystemManager()->BindKeyboardAction('6', this, &FDeferredShadingSceneRenderer::SwitchToDepthDebugMode);
	GetInputSystemManager()->BindKeyboardAction('7', this, &FDeferredShadingSceneRenderer::SwitchToDebugAllMode);
}

void FDeferredShadingSceneRenderer::CreateFrameResources(ID3D12Device* Device)
{
	CreateFrameResources_Internal<FFrameResource>(Device);
}

void FDeferredShadingSceneRenderer::BuildRootSignature()
{
	Super::BuildRootSignature();
	BuildGBufferRootSignature();
	BuildDeferredShadingPassRootSignature();


	
	D3D12_DESCRIPTOR_RANGE TextureTable = GetTextureManager()->GetTexture2DDescriptorRange();
	D3D12_DESCRIPTOR_RANGE CubeTextureTable = GetTextureManager()->GetTextureCubeDescriptorRange();

	
	constexpr UINT ROOT_PARAMETERs_NUM = 7;
	CD3DX12_ROOT_PARAMETER RootParameter[ROOT_PARAMETERs_NUM];
	RootParameter[0].InitAsConstantBufferView(0);	// PassCB
	RootParameter[1].InitAsConstantBufferView(1);	// MeshCB
	RootParameter[2].InitAsConstantBufferView(2);	// SubmeshCB
	RootParameter[3].InitAsShaderResourceView(0, 2);	// MaterialSB
	RootParameter[4].InitAsDescriptorTable(1, &TextureTable, D3D12_SHADER_VISIBILITY_PIXEL);	// TextureTable
	RootParameter[5].InitAsDescriptorTable(1, &CubeTextureTable, D3D12_SHADER_VISIBILITY_PIXEL);	// CubeTextureTable
	RootParameter[6].InitAsConstantBufferView(10);	// SkinnedCB

	FDXUtility::BuildRootSignature(RootParameter, ROOT_PARAMETERs_NUM, mRootSignatures["GBufferPass_Skinned"].GetAddressOf());
}

void FDeferredShadingSceneRenderer::BuildDeferredShadingPassRootSignature()
{
	D3D12_DESCRIPTOR_RANGE TextureTable = GetTextureManager()->GetTexture2DDescriptorRange();
	D3D12_DESCRIPTOR_RANGE CubeTextureTable = GetTextureManager()->GetTextureCubeDescriptorRange();
	constexpr UINT ROOT_PARAMETERS_NUM = 6;
	CD3DX12_ROOT_PARAMETER RootParameter[ROOT_PARAMETERS_NUM];
	RootParameter[0].InitAsDescriptorTable(1, &TextureTable, D3D12_SHADER_VISIBILITY_PIXEL);	// TextureTable
	RootParameter[1].InitAsDescriptorTable(1, &CubeTextureTable, D3D12_SHADER_VISIBILITY_PIXEL);	// CubeTextureTable
	RootParameter[2].InitAsConstantBufferView(0);	// Pass CB
	RootParameter[3].InitAsConstantBufferView(1);		// LightInfoCB
	RootParameter[4].InitAsConstantBufferView(2);		// GBufferIndices
	RootParameter[5].InitAsShaderResourceView(0, 3);	// DirLightSB

	FDXUtility::BuildRootSignature(RootParameter, ROOT_PARAMETERS_NUM, mRootSignatures["DeferredShadingPass"].GetAddressOf());
}

void FDeferredShadingSceneRenderer::BuildGBufferRootSignature()
{
	D3D12_DESCRIPTOR_RANGE TextureTable = GetTextureManager()->GetTexture2DDescriptorRange();
	D3D12_DESCRIPTOR_RANGE CubeTextureTable = GetTextureManager()->GetTextureCubeDescriptorRange();

	// Tip: 자주 사용되는 것일수록 작은 인덱스에 보관하는게 퍼포먼스가 좋음
	constexpr UINT ROOT_PARAMETERs_NUM = 6;
	CD3DX12_ROOT_PARAMETER RootParameter[ROOT_PARAMETERs_NUM];
	RootParameter[0].InitAsConstantBufferView(0);	// PassCB
	RootParameter[1].InitAsConstantBufferView(1);	// MeshCB
	RootParameter[2].InitAsConstantBufferView(2);	// SubmeshCB
	RootParameter[3].InitAsShaderResourceView(0, 2);	// MaterialSB
	RootParameter[4].InitAsDescriptorTable(1, &TextureTable, D3D12_SHADER_VISIBILITY_PIXEL);	// TextureTable
	RootParameter[5].InitAsDescriptorTable(1, &CubeTextureTable, D3D12_SHADER_VISIBILITY_PIXEL);	// CubeTextureTable

	FDXUtility::BuildRootSignature(RootParameter, ROOT_PARAMETERs_NUM, mRootSignatures["GBufferPass"].GetAddressOf());
}

void FDeferredShadingSceneRenderer::BuildShadersAndInputLayouts()
{
	Super::BuildShadersAndInputLayouts();

	BuildGBufferPassShaders();
	BuildDeferredShadingPassShaders();
	BuildDebugPassShaders();
}

void FDeferredShadingSceneRenderer::BuildDeferredShadingPassShaders()
{
	D3D_SHADER_MACRO Defines[] = {
		{"IBL", "1"},
		{NULL, NULL}
	};
	mShaders["DeferredShadingPassVertexShader"] = FDXUtility::CompileShader(
		std::wstring(PROJECT_DIR_W) + L"\\Shaders\\DeferredShadingPassVertexShader.hlsl",
		nullptr,
		"MainVS",
		"vs_5_1"
	);
	mShaders["DeferredShadingPassPixelShader"] = FDXUtility::CompileShader(
		std::wstring(PROJECT_DIR_W) + L"\\Shaders\\DeferredShadingPassPixelShader.hlsl",
		Defines,
		"MainPS",
		"ps_5_1"
	);
}

void FDeferredShadingSceneRenderer::BuildGBufferPassShaders()
{
	mShaders["GBufferPassVertexShader"] = FDXUtility::CompileShader(
		std::wstring(PROJECT_DIR_W) + L"\\Shaders\\GBufferPassVertexShader.hlsl",
		nullptr,
		"MainVS",
		"vs_5_1"
	);
	mShaders["GBufferPassPixelShader"] = FDXUtility::CompileShader(
		std::wstring(PROJECT_DIR_W) + L"\\Shaders\\GBufferPassPixelShader.hlsl",
		nullptr,
		"MainPS",
		"ps_5_1"
	);

	// TODO: SkinnedMesh 테스트용
	D3D_SHADER_MACRO Defines[] = {
		{"SKINNED", "1"},
		{NULL, NULL}
	};

	mShaders["GBufferPassVertexShader_Skinned"] = FDXUtility::CompileShader(
		std::wstring(PROJECT_DIR_W) + L"\\Shaders\\GBufferPassVertexShader.hlsl",
		Defines,
		"MainVS",
		"vs_5_1"
	);
}

void FDeferredShadingSceneRenderer::BuildDebugPassShaders()
{
	mShaders["DebugDepthPassPixelShader"] = FDXUtility::CompileShader(
		std::wstring(PROJECT_DIR_W) + L"\\Shaders\\DebugDepthPassPixelShader.hlsl",
		nullptr,
		"MainPS",
		"ps_5_1"
	);

	mShaders["DebugVectorPassPixelShader"] = FDXUtility::CompileShader(
		std::wstring(PROJECT_DIR_W) + L"\\Shaders\\DebugVectorPassPixelShader.hlsl",
		nullptr,
		"MainPS",
		"ps_5_1"
	);
}

void FDeferredShadingSceneRenderer::BuildPipelineStates(ID3D12Device* Device)
{
	Super::BuildPipelineStates(Device);
	BuildGBufferPassPipelineState(Device);
	BuildDeferredShadingPassPipelineState(Device);
	BuildDebugPassPipelineStates(Device);

	// TODO: SkinnedMesh 테스트용
	D3D12_GRAPHICS_PIPELINE_STATE_DESC GBufferPassPipelineStateDesc;
	ZeroMemory(&GBufferPassPipelineStateDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
	GBufferPassPipelineStateDesc.InputLayout = { mInputLayouts["MeshGeometryPass_Skinned"].data(), (UINT)mInputLayouts["MeshGeometryPass_Skinned"].size() };
	GBufferPassPipelineStateDesc.pRootSignature = mRootSignatures["GBufferPass_Skinned"].Get();
	GBufferPassPipelineStateDesc.VS =
	{
		reinterpret_cast<BYTE*>(mShaders["GBufferPassVertexShader_Skinned"]->GetBufferPointer()),
		mShaders["GBufferPassVertexShader_Skinned"]->GetBufferSize()
	};
	GBufferPassPipelineStateDesc.PS =
	{
		reinterpret_cast<BYTE*>(mShaders["GBufferPassPixelShader"]->GetBufferPointer()),
		mShaders["GBufferPassPixelShader"]->GetBufferSize()
	};
	GBufferPassPipelineStateDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	GBufferPassPipelineStateDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	GBufferPassPipelineStateDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	GBufferPassPipelineStateDesc.SampleMask = UINT_MAX;
	GBufferPassPipelineStateDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	GBufferPassPipelineStateDesc.NumRenderTargets = 3;
	GBufferPassPipelineStateDesc.RTVFormats[0] = mGBufferA->GetFormat();
	GBufferPassPipelineStateDesc.RTVFormats[1] = mGBufferB->GetFormat();
	GBufferPassPipelineStateDesc.RTVFormats[2] = mGBufferC->GetFormat();
	GBufferPassPipelineStateDesc.SampleDesc.Count = 1;
	GBufferPassPipelineStateDesc.SampleDesc.Quality = 0;
	GBufferPassPipelineStateDesc.DSVFormat = mGBufferDepthStencil->GetFormat();
	THROW_IF_FAILED(
		Device->CreateGraphicsPipelineState(
			&GBufferPassPipelineStateDesc,
			IID_PPV_ARGS(mPipelineStates["GBufferPass_Skinned"].GetAddressOf())
		)
	);
}

void FDeferredShadingSceneRenderer::BuildDeferredShadingPassPipelineState(ID3D12Device* Device)
{
	D3D12_GRAPHICS_PIPELINE_STATE_DESC DeferredShadingPassPipelineStateDesc;
	ZeroMemory(&DeferredShadingPassPipelineStateDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
	std::vector<D3D12_INPUT_ELEMENT_DESC> InputLayout = GetDrawingRectInputLayouts();
	DeferredShadingPassPipelineStateDesc.InputLayout = { InputLayout.data(), (UINT)InputLayout.size() };
	DeferredShadingPassPipelineStateDesc.pRootSignature = mRootSignatures["DeferredShadingPass"].Get();
	DeferredShadingPassPipelineStateDesc.VS =
	{
		reinterpret_cast<BYTE*>(mShaders["DeferredShadingPassVertexShader"]->GetBufferPointer()),
		mShaders["DeferredShadingPassVertexShader"]->GetBufferSize()
	};
	DeferredShadingPassPipelineStateDesc.PS =
	{
		reinterpret_cast<BYTE*>(mShaders["DeferredShadingPassPixelShader"]->GetBufferPointer()),
		mShaders["DeferredShadingPassPixelShader"]->GetBufferSize()
	};
	DeferredShadingPassPipelineStateDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	DeferredShadingPassPipelineStateDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	DeferredShadingPassPipelineStateDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	DeferredShadingPassPipelineStateDesc.SampleMask = UINT_MAX;
	DeferredShadingPassPipelineStateDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	DeferredShadingPassPipelineStateDesc.NumRenderTargets = 1;
	DeferredShadingPassPipelineStateDesc.RTVFormats[0] = GetDXResourceManagerPtr()->GetBackbufferFormat();
	DeferredShadingPassPipelineStateDesc.SampleDesc.Count = 1;
	DeferredShadingPassPipelineStateDesc.SampleDesc.Quality = 0;
	DeferredShadingPassPipelineStateDesc.DSVFormat = GetDXResourceManagerPtr()->GetDepthStencilFormat();
	THROW_IF_FAILED(
		Device->CreateGraphicsPipelineState(
			&DeferredShadingPassPipelineStateDesc,
			IID_PPV_ARGS(mPipelineStates["DeferredShadingPass"].GetAddressOf())
		)
	);
}

void FDeferredShadingSceneRenderer::BuildGBufferPassPipelineState(ID3D12Device* Device)
{
	D3D12_GRAPHICS_PIPELINE_STATE_DESC GBufferPassPipelineStateDesc;
	ZeroMemory(&GBufferPassPipelineStateDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
	GBufferPassPipelineStateDesc.InputLayout = { mInputLayouts["MeshGeometryPass"].data(), (UINT)mInputLayouts["MeshGeometryPass"].size() };
	GBufferPassPipelineStateDesc.pRootSignature = mRootSignatures["GBufferPass"].Get();
	GBufferPassPipelineStateDesc.VS =
	{
		reinterpret_cast<BYTE*>(mShaders["GBufferPassVertexShader"]->GetBufferPointer()),
		mShaders["GBufferPassVertexShader"]->GetBufferSize()
	};
	GBufferPassPipelineStateDesc.PS =
	{
		reinterpret_cast<BYTE*>(mShaders["GBufferPassPixelShader"]->GetBufferPointer()),
		mShaders["GBufferPassPixelShader"]->GetBufferSize()
	};
	GBufferPassPipelineStateDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	GBufferPassPipelineStateDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	GBufferPassPipelineStateDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	GBufferPassPipelineStateDesc.SampleMask = UINT_MAX;
	GBufferPassPipelineStateDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	GBufferPassPipelineStateDesc.NumRenderTargets = 3;
	GBufferPassPipelineStateDesc.RTVFormats[0] = mGBufferA->GetFormat();
	GBufferPassPipelineStateDesc.RTVFormats[1] = mGBufferB->GetFormat();
	GBufferPassPipelineStateDesc.RTVFormats[2] = mGBufferC->GetFormat();
	GBufferPassPipelineStateDesc.SampleDesc.Count = 1;
	GBufferPassPipelineStateDesc.SampleDesc.Quality = 0;
	GBufferPassPipelineStateDesc.DSVFormat = mGBufferDepthStencil->GetFormat();
	THROW_IF_FAILED(
		Device->CreateGraphicsPipelineState(
			&GBufferPassPipelineStateDesc,
			IID_PPV_ARGS(mPipelineStates["GBufferPass"].GetAddressOf())
		)
	);
}

void FDeferredShadingSceneRenderer::BuildDebugPassPipelineStates(ID3D12Device* Device)
{
	D3D12_GRAPHICS_PIPELINE_STATE_DESC DebugPassPipelineStateBase;
	ZeroMemory(&DebugPassPipelineStateBase, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
	std::vector<D3D12_INPUT_ELEMENT_DESC> InputLayout = GetDrawingRectInputLayouts();
	DebugPassPipelineStateBase.InputLayout = { InputLayout.data(), (UINT)InputLayout.size() };
	DebugPassPipelineStateBase.pRootSignature = mRootSignatures["DrawRectPass"].Get();
	DebugPassPipelineStateBase.VS =
	{
		reinterpret_cast<BYTE*>(mShaders["DrawRectPassVertexShader"]->GetBufferPointer()),
		mShaders["DrawRectPassVertexShader"]->GetBufferSize()
	};
	DebugPassPipelineStateBase.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	DebugPassPipelineStateBase.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	DebugPassPipelineStateBase.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	DebugPassPipelineStateBase.SampleMask = UINT_MAX;
	DebugPassPipelineStateBase.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	DebugPassPipelineStateBase.NumRenderTargets = 1;
	DebugPassPipelineStateBase.RTVFormats[0] = GetDXResourceManagerPtr()->GetBackbufferFormat();
	DebugPassPipelineStateBase.SampleDesc.Count = 1;
	DebugPassPipelineStateBase.SampleDesc.Quality = 0;
	DebugPassPipelineStateBase.DSVFormat = GetDXResourceManagerPtr()->GetDepthStencilFormat();

	// DebugDepthPassPixelShader
	DebugPassPipelineStateBase.PS =
	{
		reinterpret_cast<BYTE*>(mShaders["DebugDepthPassPixelShader"]->GetBufferPointer()),
		mShaders["DebugDepthPassPixelShader"]->GetBufferSize()
	};
	THROW_IF_FAILED(
		Device->CreateGraphicsPipelineState(
			&DebugPassPipelineStateBase,
			IID_PPV_ARGS(mPipelineStates["DebugDepthPass"].GetAddressOf())
		)
	);

	// DebugVectorPassPixelShader
	DebugPassPipelineStateBase.PS =
	{
		reinterpret_cast<BYTE*>(mShaders["DebugVectorPassPixelShader"]->GetBufferPointer()),
		mShaders["DebugVectorPassPixelShader"]->GetBufferSize()
	};
	THROW_IF_FAILED(
		Device->CreateGraphicsPipelineState(
			&DebugPassPipelineStateBase,
			IID_PPV_ARGS(mPipelineStates["DebugVectorPass"].GetAddressOf())
		)
	);
}

void FDeferredShadingSceneRenderer::UpdateFrameBuffers(FFrameResourceBase* FrameResource, const FRenderItemProxy* RenderItemProxy)
{
	Super::UpdateFrameBuffers(FrameResource, RenderItemProxy);

	FFrameResource* Fr = dynamic_cast<FFrameResource*>(FrameResource);
}

void FDeferredShadingSceneRenderer::SwitchToDefaultMode()
{
	mScreenMode = EDebugScreenMode::EDSM_Default;
}

void FDeferredShadingSceneRenderer::SwitchToDebugAllMode()
{
	mScreenMode = EDebugScreenMode::EDSM_DebugAll;
}

void FDeferredShadingSceneRenderer::SwitchToBlurMode()
{
	mScreenMode = EDebugScreenMode::EDSM_Blur;
}

void FDeferredShadingSceneRenderer::SwitchToGBufferADebugMode()
{
	mScreenMode = EDebugScreenMode::EDSM_GBufferA;
}

void FDeferredShadingSceneRenderer::SwitchToGBufferBDebugMode()
{
	mScreenMode = EDebugScreenMode::EDSM_GBufferB;
}

void FDeferredShadingSceneRenderer::SwitchToGBufferCDebugMode()
{
	mScreenMode = EDebugScreenMode::EDSM_GBufferC;
}

void FDeferredShadingSceneRenderer::SwitchToDepthDebugMode()
{
	mScreenMode = EDebugScreenMode::EDSM_Depth;
}

void FDeferredShadingSceneRenderer::UpdateDeferredShadingPassCB(TUploadBuffer<FDeferredShadingPassConstantBuffer>* DeferredShadingPassCB)
{
	FDeferredShadingPassConstantBuffer CB;
	CB.GBufferAIndex = mGBufferA->GetSRVHeapIndex();
	CB.GBufferBIndex = mGBufferB->GetSRVHeapIndex();
	CB.GBufferCIndex = mGBufferC->GetSRVHeapIndex();
	CB.DepthBufferIndex = mGBufferDepthStencil->GetSRVHeapIndex();
	CB.PrefilteredMapIndex = mEnvironmentMapRenderer->GetPrefilteredMapRenderTarget()->GetSRVHeapIndex();
	CB.IrradianceMapIndex = mEnvironmentMapRenderer->GetIrradianceMapRenderTarget()->GetSRVHeapIndex();
	CB.BRDFLUTIndex = mEnvironmentMapRenderer->GetBRDFLUTRenderTarget()->GetSRVHeapIndex();
	DeferredShadingPassCB->CopyData(0, CB);
}

void FDeferredShadingSceneRenderer::Render(
	ID3D12GraphicsCommandList* CommandList,
	FFrameResourceBase* FrameResourceBase,
	FResource* Resource,
	D3D12_CPU_DESCRIPTOR_HANDLE Rtv,
	D3D12_CPU_DESCRIPTOR_HANDLE Dsv,
	D3D12_VIEWPORT Viewport,
	D3D12_RECT ScissorRect,
	const FRenderItemProxy* RenderItemProxy
)
{
	FFrameResource* FrameResource = dynamic_cast<FFrameResource*>(FrameResourceBase);

	DrawGBuffers(CommandList, FrameResource, RenderItemProxy);
	DrawShadowMap(CommandList, FrameResource, RenderItemProxy);

	ReadyBackBuffer(CommandList);
	CommandList->OMSetRenderTargets(1, &Rtv, true, &Dsv);
	CommandList->RSSetViewports(1, &Viewport);
	CommandList->RSSetScissorRects(1, &ScissorRect);

	DrawDeferredShadingPass(
		CommandList,
		Rtv,
		Dsv,
		Viewport,
		FrameResource
	);

	mEnvironmentMapRenderer->Render(CommandList, Rtv, Dsv, Viewport);

	switch (mScreenMode)
	{
	case EDebugScreenMode::EDSM_Blur:
		mGaussianBlurFilter->Execute(CommandList, Resource, 10);
		break;

	case EDebugScreenMode::EDSM_GBufferA:
		DrawRectPass(CommandList, mGBufferA->GetSRVHeapIndex(), Rtv, Dsv, Viewport);
		break;

	case EDebugScreenMode::EDSM_GBufferB:
		DrawRectPass(CommandList, mGBufferB->GetSRVHeapIndex(), Rtv, Dsv, Viewport);
		break;

	case EDebugScreenMode::EDSM_GBufferC:
		DrawRectPass(CommandList, mGBufferC->GetSRVHeapIndex(), Rtv, Dsv, Viewport);
		break;

	case EDebugScreenMode::EDSM_Depth:
		DrawRectPass(CommandList, mGBufferDepthStencil->GetSRVHeapIndex(), Rtv, Dsv, Viewport, "DebugDepthPass");
		break;

	case EDebugScreenMode::EDSM_DebugAll:
	{
		D3D12_VIEWPORT DebugScreenViewport = FDXUtility::GetQuadrantViewport(Viewport, 2);
		DebugScreenViewport = FDXUtility::GetQuadrantViewport(DebugScreenViewport, 2);
		DrawDebugGBuffers(CommandList, Rtv, Dsv, DebugScreenViewport);
		DrawRectPass(
			CommandList,
			mGBufferDepthStencil->GetSRVHeapIndex(),
			Rtv,
			Dsv,
			FDXUtility::GetQuadrantViewport(DebugScreenViewport, 4),
			"DebugDepthPass"
		);
		break;
	}

	default:
		break;
	}

	// DrawDebugLine
	mGBufferDepthStencil->TransitionResourceBarrier(CommandList, D3D12_RESOURCE_STATE_DEPTH_WRITE);

	D3D12_CPU_DESCRIPTOR_HANDLE GBufferDsv = mGBufferDepthStencil->GetDSV();
	CommandList->OMSetRenderTargets(1, &Rtv, false, &GBufferDsv);

	CommandList->RSSetViewports(1, &Viewport);
	CommandList->RSSetScissorRects(1, &ScissorRect);

	auto* Line3DVB = FrameResource->mLine3DVB.get();
	UINT IndexCount = FrameResource->mLine3DVB->GetElementCount();

	// 3. Vertex Buffer View 설정
	D3D12_VERTEX_BUFFER_VIEW vbv;
	vbv.BufferLocation = Line3DVB->Resource()->GetGPUVirtualAddress();
	vbv.SizeInBytes = IndexCount * sizeof(FLine3DVertex);
	vbv.StrideInBytes = sizeof(FLine3DVertex);

	// 4. Command List 설정 및 드로우
	CommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_LINELIST);
	CommandList->IASetVertexBuffers(0, 1, &vbv);

	// RootSignature와 PSO 바인딩은 생략됨
	CommandList->SetGraphicsRootSignature(mRootSignatures["DrawDebugLine3DPass"].Get());
	CommandList->SetPipelineState(mPipelineStates["DrawDebugLine3DPass"].Get());
	CommandList->SetGraphicsRootConstantBufferView(0, FrameResource->GetPassCB()->Resource()->GetGPUVirtualAddress());

	CommandList->DrawInstanced(IndexCount, 1, 0, 0);

	
	FinishBackBuffer(CommandList);
}

void FDeferredShadingSceneRenderer::DrawDebugGBuffers(
	ID3D12GraphicsCommandList* CommandList,
	D3D12_CPU_DESCRIPTOR_HANDLE Rtv,
	D3D12_CPU_DESCRIPTOR_HANDLE Dsv,
	const D3D12_VIEWPORT& Viewport
)
{
	D3D12_RECT ScissorRect = FDXUtility::MakeScissorRectFromViewport(Viewport);
	CommandList->RSSetViewports(1, &Viewport);
	CommandList->RSSetScissorRects(1, &ScissorRect);
	CommandList->ClearRenderTargetView(
		Rtv,
		DirectX::Colors::Black,
		1,
		&ScissorRect
	);
	CommandList->ClearDepthStencilView(
		Dsv,
		D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL,
		1.0f,
		0,
		1,
		&ScissorRect
	);
	CommandList->OMSetRenderTargets(1, &Rtv, false, &Dsv);

	// 1사분면: GBufferA	
	DrawRectPass(
		CommandList,
		mGBufferA->GetSRVHeapIndex(),
		Rtv,
		Dsv,
		FDXUtility::GetQuadrantViewport(Viewport, 1),
		"DebugVectorPass"
	);
	// 2사분면: GBufferB
	DrawRectPass(
		CommandList,
		mGBufferB->GetSRVHeapIndex(),
		Rtv,
		Dsv,
		FDXUtility::GetQuadrantViewport(Viewport, 2)
	);
	// 3사분면: GBufferC
	DrawRectPass(
		CommandList,
		mGBufferC->GetSRVHeapIndex(),
		Rtv,
		Dsv,
		FDXUtility::GetQuadrantViewport(Viewport, 3)
	);
}

void FDeferredShadingSceneRenderer::DrawDeferredShadingPass(
	ID3D12GraphicsCommandList* CommandList,
	D3D12_CPU_DESCRIPTOR_HANDLE Rtv,
	D3D12_CPU_DESCRIPTOR_HANDLE Dsv,
	const D3D12_VIEWPORT& Viewport,
	FFrameResource* FrameResource
)
{
	CommandList->RSSetViewports(1, &Viewport);
	D3D12_RECT ScissorRect = FDXUtility::MakeScissorRectFromViewport(Viewport);
	CommandList->RSSetScissorRects(1, &ScissorRect);

	CommandList->ClearRenderTargetView(
		Rtv,
		DirectX::Colors::Black,
		1,
		&ScissorRect
	);
	CommandList->ClearDepthStencilView(
		Dsv,
		D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL,
		1.0f,
		0,
		1,
		&ScissorRect
	);
	CommandList->OMSetRenderTargets(1, &Rtv, false, &Dsv);

	CommandList->SetPipelineState(mPipelineStates["DeferredShadingPass"].Get());

	CommandList->SetGraphicsRootSignature(mRootSignatures["DeferredShadingPass"].Get());

	FCBVSRVUAVHeap* SRVHeap = GetCBVSRVUAVHeap();
	CommandList->SetGraphicsRootDescriptorTable(0, SRVHeap->GetTexture2DGPUDescriptorHandleStart());
	CommandList->SetGraphicsRootDescriptorTable(1, SRVHeap->GetTextureCubeGPUDescriptorHandleStart());
	CommandList->SetGraphicsRootConstantBufferView(2, FrameResource->GetPassCB()->Resource()->GetGPUVirtualAddress());
	CommandList->SetGraphicsRootConstantBufferView(3, FrameResource->GetLightInfoCB()->Resource()->GetGPUVirtualAddress());
	CommandList->SetGraphicsRootConstantBufferView(4, FrameResource->GetDeferredShadingPassCB()->Resource()->GetGPUVirtualAddress());
	CommandList->SetGraphicsRootShaderResourceView(5, FrameResource->GetDirectionalLightSB()->Resource()->GetGPUVirtualAddress());
	DrawRect(CommandList);
}

void FDeferredShadingSceneRenderer::DrawGBuffers(ID3D12GraphicsCommandList* CommandList, FFrameResource* FrameResource, const FRenderItemProxy* RenderItemProxy)
{
	mGBufferA->TransitionResourceBarrier(CommandList, D3D12_RESOURCE_STATE_RENDER_TARGET);
	mGBufferB->TransitionResourceBarrier(CommandList, D3D12_RESOURCE_STATE_RENDER_TARGET);
	mGBufferC->TransitionResourceBarrier(CommandList, D3D12_RESOURCE_STATE_RENDER_TARGET);
	mGBufferDepthStencil->TransitionResourceBarrier(CommandList, D3D12_RESOURCE_STATE_DEPTH_WRITE);
	mGBufferA->Clear(CommandList);
	mGBufferB->Clear(CommandList);
	mGBufferC->Clear(CommandList);
	mGBufferDepthStencil->Clear(CommandList);

	std::vector<D3D12_CPU_DESCRIPTOR_HANDLE> GBufferRTVs;
	GBufferRTVs.push_back(mGBufferA->GetRTV(0));
	GBufferRTVs.push_back(mGBufferB->GetRTV(0));
	GBufferRTVs.push_back(mGBufferC->GetRTV(0));
	D3D12_CPU_DESCRIPTOR_HANDLE GBufferDsv = mGBufferDepthStencil->GetDSV();
	CommandList->OMSetRenderTargets(3, GBufferRTVs.data(), false, &GBufferDsv);

	D3D12_VIEWPORT GBffuerViewport = mGBufferA->GetViewport();
	D3D12_RECT GBufferScissoreRect = mGBufferA->GetScissorRect();
	CommandList->RSSetViewports(1, &GBffuerViewport);
	CommandList->RSSetScissorRects(1, &GBufferScissoreRect);

	CommandList->SetPipelineState(mPipelineStates["GBufferPass"].Get());

	CommandList->SetGraphicsRootSignature(mRootSignatures["GBufferPass"].Get());

	FCBVSRVUAVHeap* SRVHeap = GetCBVSRVUAVHeap();
	ID3D12DescriptorHeap* DescriptorHeaps[] = { SRVHeap->Get() };
	CommandList->SetDescriptorHeaps(_countof(DescriptorHeaps), DescriptorHeaps);

	ID3D12Resource* PassCB = FrameResource->GetPassCB()->Resource();
	CommandList->SetGraphicsRootConstantBufferView(0, PassCB->GetGPUVirtualAddress());
	// 1 : Mesh, 2 : Submesh 생략. DrawRenderItems에서 설정됨.
	ID3D12Resource* MaterialSB = FrameResource->GetMaterialSB()->Resource();
	CommandList->SetGraphicsRootShaderResourceView(3, MaterialSB->GetGPUVirtualAddress());
	CommandList->SetGraphicsRootDescriptorTable(4, SRVHeap->GetTexture2DGPUDescriptorHandleStart());
	CommandList->SetGraphicsRootDescriptorTable(5, SRVHeap->GetTextureCubeGPUDescriptorHandleStart());

	DrawStaticMeshs(
		CommandList,
		FrameResource->GetMeshCB()->Resource(),
		FrameResource->GetSubmeshCB()->Resource(),
		RenderItemProxy
	);

	////////////////////////////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////////////
	// TODO: SkinnedMesh 테스트용

	CommandList->SetPipelineState(mPipelineStates["GBufferPass_Skinned"].Get());

	CommandList->SetGraphicsRootSignature(mRootSignatures["GBufferPass_Skinned"].Get());

	//CommandList->SetDescriptorHeaps(_countof(DescriptorHeaps), DescriptorHeaps);
	//CommandList->SetGraphicsRootConstantBufferView(0, PassCB->GetGPUVirtualAddress());
	//CommandList->SetGraphicsRootShaderResourceView(3, MaterialSB->GetGPUVirtualAddress());
	//CommandList->SetGraphicsRootDescriptorTable(4, SRVHeap->GetTexture2DGPUDescriptorHandleStart());
	//CommandList->SetGraphicsRootDescriptorTable(5, SRVHeap->GetTextureCubeGPUDescriptorHandleStart());

	//UINT ObjectConstantBufferByteSize = FDXUtility::CalcConstantBufferByteSize(sizeof(FMeshConstantBuffer));
	//UINT SubmeshConstantBufferByteSize = FDXUtility::CalcConstantBufferByteSize(sizeof(FSubmeshConstantBuffer));
	//UINT SkinnedConstantBufferByteSize = FDXUtility::CalcConstantBufferByteSize(sizeof(FSkinnedConstantBuffer));

	//ID3D12Resource* MeshConstantBuffer = FrameResource->GetMeshCB()->Resource();
	//ID3D12Resource* SubmeshConstantBuffer = FrameResource->GetSubmeshCB()->Resource();

	//// MeshConstantBuffer
	//auto ObjectConstantBufferAddress = MeshConstantBuffer->GetGPUVirtualAddress() + StaticMeshInfo.MeshCBIndex * ObjectConstantBufferByteSize;
	//CommandList->SetGraphicsRootConstantBufferView(1, ObjectConstantBufferAddress);

	//// SubmeshConstantBuffer
	//auto SubmeshConstantBufferAddress = SubmeshConstantBuffer->GetGPUVirtualAddress() + StaticMeshInfo.SubmeshCBIndex * SubmeshConstantBufferByteSize;
	//CommandList->SetGraphicsRootConstantBufferView(2, SubmeshConstantBufferAddress);

	// SkinnedConstantBuffer
	auto SKinnedConstantBufferAddress = FrameResource->GetSkinnedCB()->Resource()->GetGPUVirtualAddress();
	CommandList->SetGraphicsRootConstantBufferView(6, SKinnedConstantBufferAddress);

	auto MeshGeo = FMeshGeometryManager::GetInstance()->GetMeshGeometry("Soldier");

	D3D12_VERTEX_BUFFER_VIEW VertexBufferView = MeshGeo->VertexBufferView();
	D3D12_INDEX_BUFFER_VIEW IndexBufferView = MeshGeo->IndexBufferView();
	CommandList->IASetVertexBuffers(0, 1, &VertexBufferView);
	CommandList->IASetIndexBuffer(&IndexBufferView);
	CommandList->IASetPrimitiveTopology(MeshGeo->PrimitiveType);

	//for (int i = 0; i < MeshGeo->DrawArgs.size(); ++i)
	//{
	//	CommandList->DrawIndexedInstanced(
	//		MeshGeo->DrawArgs[i].IndexCount,
	//		1,
	//		MeshGeo->DrawArgs[i].StartIndexLocation,
	//		MeshGeo->DrawArgs[i].BaseVertexLocation,
	//		0
	//	);
	//}

	

	////////////////////////////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////////////

	mGBufferA->TransitionResourceBarrier(CommandList, D3D12_RESOURCE_STATE_GENERIC_READ);
	mGBufferB->TransitionResourceBarrier(CommandList, D3D12_RESOURCE_STATE_GENERIC_READ);
	mGBufferC->TransitionResourceBarrier(CommandList, D3D12_RESOURCE_STATE_GENERIC_READ);
	mGBufferDepthStencil->TransitionResourceBarrier(CommandList, D3D12_RESOURCE_STATE_DEPTH_READ);
}
