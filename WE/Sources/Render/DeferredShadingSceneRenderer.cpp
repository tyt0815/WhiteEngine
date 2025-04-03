#include "DeferredShadingSceneRenderer.h"
#include "DirectX/DXResourceManager.h"

FDeferredShadingSceneRenderer::FFrameResource::FFrameResource(ID3D12Device* Device) :
	FFrameResourceBase(Device)
{

}

FDeferredShadingSceneRenderer::FDeferredShadingSceneRenderer()
{
	mGBuffers = std::make_unique<FRenderTarget>(
		"GBuffers",
		(UINT)GetDXResourceManagerPtr()->GetScreenViewport().Width,
		(UINT)GetDXResourceManagerPtr()->GetScreenViewport().Height,
		3u,
		1u
	);
}

void FDeferredShadingSceneRenderer::Initialize(ID3D12Device* Device)
{
	Super::Initialize(Device);
}

void FDeferredShadingSceneRenderer::CreateFrameResources(ID3D12Device* Device)
{
	CreateFrameResources_Internal<FFrameResource>(Device);
}

void FDeferredShadingSceneRenderer::BuildRootSignature()
{
	BuildGBufferRootSignature();
	BuildDeferredShadingPassRootSignature();
}

void FDeferredShadingSceneRenderer::BuildDeferredShadingPassRootSignature()
{
	D3D12_DESCRIPTOR_RANGE TextureTable = GetTextureManager()->GetTexture2DDescriptorRange();
	D3D12_DESCRIPTOR_RANGE CubeTextureTable = GetTextureManager()->GetTextureCubeDescriptorRange();
	constexpr UINT ROOT_PARAMETERS_NUM = 2;
	CD3DX12_ROOT_PARAMETER RootParameter[ROOT_PARAMETERS_NUM];
	RootParameter[0].InitAsDescriptorTable(1, &TextureTable, D3D12_SHADER_VISIBILITY_PIXEL);	// TextureTable
	RootParameter[1].InitAsDescriptorTable(1, &CubeTextureTable, D3D12_SHADER_VISIBILITY_PIXEL);	// CubeTextureTable

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
	D3D_SHADER_MACRO Defines[] = {
		{NULL, NULL}
	};

	BuildGBufferPassShaders();
	BuildDeferredShadingPassShaders();

	mInputLayouts["MeshPass"] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
	};
}

void FDeferredShadingSceneRenderer::BuildDeferredShadingPassShaders()
{
	mShaders["DeferredShadingPassVertexShader"] = FDXUtility::CompileShader(
		L"Shaders\\DeferredShadingPassVertexShader.sf",
		nullptr,
		"MainVS",
		"vs_5_1"
	);
	mShaders["DeferredShadingPassPixelShader"] = FDXUtility::CompileShader(
		L"Shaders\\DeferredShadingPassPixelShader.sf",
		nullptr,
		"MainPS",
		"ps_5_1"
	);
}

void FDeferredShadingSceneRenderer::BuildGBufferPassShaders()
{
	mShaders["GBufferPassVertexShader"] = FDXUtility::CompileShader(
		L"Shaders\\GBufferPassVertexShader.sf",
		nullptr,
		"MainVS",
		"vs_5_1"
	);
	mShaders["GBufferPassPixelShader"] = FDXUtility::CompileShader(
		L"Shaders\\GBufferPassPixelShader.sf",
		nullptr,
		"MainPS",
		"ps_5_1"
	);
}

void FDeferredShadingSceneRenderer::BuildPipelineStates(ID3D12Device* Device)
{
	D3D12_GRAPHICS_PIPELINE_STATE_DESC GBufferPassPipelineStateDesc;
	ZeroMemory(&GBufferPassPipelineStateDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
	GBufferPassPipelineStateDesc.InputLayout = { mInputLayouts["MeshPass"].data(), (UINT)mInputLayouts["MeshPass"].size() };
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
	GBufferPassPipelineStateDesc.NumRenderTargets = 1;
	GBufferPassPipelineStateDesc.RTVFormats[0] = mGBuffers->GetFormat();
	GBufferPassPipelineStateDesc.SampleDesc.Count = 1;
	GBufferPassPipelineStateDesc.SampleDesc.Quality = 0;
	GBufferPassPipelineStateDesc.DSVFormat = mGBuffers->GetDepthStencilFormat();
	THROW_IF_FAILED(
		Device->CreateGraphicsPipelineState(
			&GBufferPassPipelineStateDesc,
			IID_PPV_ARGS(mPipelineStates["GBufferPass"].GetAddressOf())
		)
	);
}

void FDeferredShadingSceneRenderer::UpdateFrameBuffers(FFrameResourceBase* FrameResource)
{
	Super::UpdateFrameBuffers(FrameResource);
}

void FDeferredShadingSceneRenderer::Render(ID3D12GraphicsCommandList* CommandList, FFrameResourceBase* FrameResource)
{
	ID3D12Resource* BackBuffer = GetDXResourceManagerPtr()->GetCurrentBackBufferPtr();
	D3D12_CPU_DESCRIPTOR_HANDLE BackBufferView = GetDXResourceManagerPtr()->GetCurrentBackBufferView();
	D3D12_CPU_DESCRIPTOR_HANDLE BackBufferDsv = GetDXResourceManagerPtr()->GetDepthStencilView();
	D3D12_VIEWPORT BackBufferViewport = GetDXResourceManagerPtr()->GetScreenViewport();
	D3D12_RECT BackBufferScissorRect = GetDXResourceManagerPtr()->GetScissorRect();

	ReadyRednerTarget(
		CommandList,
		BackBuffer,
		BackBufferView,
		BackBufferDsv,
		BackBufferViewport,
		BackBufferScissorRect
	);

	

	FinishRenderTarget(
		CommandList,
		BackBuffer
	);
}