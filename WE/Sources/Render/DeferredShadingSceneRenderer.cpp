#include "DeferredShadingSceneRenderer.h"
#include "DirectX/DXResourceManager.h"
#include "ShapeDrawer.h"

#include <vector>

FDeferredShadingSceneRenderer::FFrameResource::FFrameResource(ID3D12Device* Device) :
	FFrameResourceBase(Device)
{
	mGBufferInfoCB = std::make_unique<TUploadBuffer<FGBufferInfoConstantBuffer>>(Device, 1, true);
}

FDeferredShadingSceneRenderer::FDeferredShadingSceneRenderer()
{
	std::vector<std::string> Names;
	Names.push_back("GBufferA");
	Names.push_back("GBufferB");
	Names.push_back("GBufferC");
	mGBuffers = std::make_unique<FRenderTarget>(
		Names,
		(UINT)GetDXResourceManagerPtr()->GetScreenViewport().Width,
		(UINT)GetDXResourceManagerPtr()->GetScreenViewport().Height
	);
}

void FDeferredShadingSceneRenderer::Initialize(ID3D12Device* Device)
{
	Super::Initialize(Device);
	
	for (int i = 0; i < mFrameResources.size(); ++i)
	{
		FFrameResource* FrameResource = dynamic_cast<FFrameResource*>(mFrameResources[i].get());
		UpdateGBufferInfoCB(FrameResource->GetGBufferInfoCB());
	}
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
}

void FDeferredShadingSceneRenderer::BuildDeferredShadingPassRootSignature()
{
	D3D12_DESCRIPTOR_RANGE TextureTable = GetTextureManager()->GetTexture2DDescriptorRange();
	D3D12_DESCRIPTOR_RANGE CubeTextureTable = GetTextureManager()->GetTextureCubeDescriptorRange();
	constexpr UINT ROOT_PARAMETERS_NUM = 5;
	CD3DX12_ROOT_PARAMETER RootParameter[ROOT_PARAMETERS_NUM];
	RootParameter[0].InitAsDescriptorTable(1, &TextureTable, D3D12_SHADER_VISIBILITY_PIXEL);	// TextureTable
	RootParameter[1].InitAsDescriptorTable(1, &CubeTextureTable, D3D12_SHADER_VISIBILITY_PIXEL);	// CubeTextureTable
	RootParameter[2].InitAsConstantBufferView(0);	// Pass CB
	RootParameter[3].InitAsConstantBufferView(1);			// GBufferIndices
	RootParameter[4].InitAsShaderResourceView(0, 3);	// DirLightSB

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
	Super::BuildPipelineStates(Device);
	BuildGBufferPassPipelineState(Device);
	BuildDeferredShadingPassPipelineState(Device);
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
	GBufferPassPipelineStateDesc.NumRenderTargets = 3;
	GBufferPassPipelineStateDesc.RTVFormats[0] = mGBuffers->GetFormat();
	GBufferPassPipelineStateDesc.RTVFormats[1] = mGBuffers->GetFormat();
	GBufferPassPipelineStateDesc.RTVFormats[2] = mGBuffers->GetFormat();
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

void FDeferredShadingSceneRenderer::UpdateGBufferInfoCB(TUploadBuffer<FGBufferInfoConstantBuffer>* GBufferInfoCB)
{
	FGBufferInfoConstantBuffer GBufferCB;
	GBufferCB.GBufferAIndex = mGBuffers->GetTexture("GBufferA")->SRVHeapIndex;
	GBufferCB.GBufferBIndex = mGBuffers->GetTexture("GBufferB")->SRVHeapIndex;
	GBufferCB.GBufferCIndex = mGBuffers->GetTexture("GBufferC")->SRVHeapIndex;
	GBufferInfoCB->CopyData(0, GBufferCB);
}

void FDeferredShadingSceneRenderer::Render(ID3D12GraphicsCommandList* CommandList, FFrameResourceBase* FrameResourceBase)
{
	FFrameResource* FrameResource = dynamic_cast<FFrameResource*>(FrameResourceBase);

	DrawGBuffers(CommandList, FrameResource);

	ReadyBackBuffer(CommandList);
	D3D12_CPU_DESCRIPTOR_HANDLE BackBufferView = GetDXResourceManagerPtr()->GetCurrentBackBufferView();
	D3D12_CPU_DESCRIPTOR_HANDLE BackBufferDsv = GetDXResourceManagerPtr()->GetDepthStencilView();
	CommandList->OMSetRenderTargets(1, &BackBufferView, true, &BackBufferDsv);

	D3D12_VIEWPORT BackBufferViewport = GetDXResourceManagerPtr()->GetScreenViewport();
	D3D12_RECT BackBufferScissorRect = GetDXResourceManagerPtr()->GetScissorRect();
	CommandList->RSSetScissorRects(1, &BackBufferScissorRect);

	// 2사분면->Lit 결과
	D3D12_VIEWPORT SecondQuadrantViewport = BackBufferViewport;
	SecondQuadrantViewport.Width /= 2.0f;
	SecondQuadrantViewport.Height /= 2.0f;
	CommandList->RSSetViewports(1, &SecondQuadrantViewport);

	CommandList->SetPipelineState(mPipelineStates["DeferredShadingPass"].Get());

	CommandList->SetGraphicsRootSignature(mRootSignatures["DeferredShadingPass"].Get());

	CommandList->SetGraphicsRootDescriptorTable(0, GetTextureManager()->GetTexture2DGPUSRVForHeapStart());
	CommandList->SetGraphicsRootDescriptorTable(1, GetTextureManager()->GetTextureCubeGPUSRVForHeapStart());
	CommandList->SetGraphicsRootConstantBufferView(2, FrameResource->GetPassCB()->Resource()->GetGPUVirtualAddress());
	CommandList->SetGraphicsRootConstantBufferView(3, FrameResource->GetGBufferInfoCB()->Resource()->GetGPUVirtualAddress());
	CommandList->SetGraphicsRootShaderResourceView(4, FrameResource->GetDirectionalLightSB()->Resource()->GetGPUVirtualAddress());
	DrawRect(CommandList);

	UINT GBufferATextureIndex = mGBuffers->GetTexture("GBufferA")->SRVHeapIndex;
	UINT GBufferBTextureIndex = mGBuffers->GetTexture("GBufferB")->SRVHeapIndex;
	UINT GBufferCTextureIndex = mGBuffers->GetTexture("GBufferC")->SRVHeapIndex;

	// 1사분면->GBufferA
	D3D12_VIEWPORT FirstQuadrantViewport = SecondQuadrantViewport;
	FirstQuadrantViewport.TopLeftX = FirstQuadrantViewport.Width;
	CommandList->RSSetViewports(1, &FirstQuadrantViewport);

	CommandList->SetPipelineState(mPipelineStates["DrawRectPass"].Get());

	CommandList->SetGraphicsRootSignature(mRootSignatures["DrawRectPass"].Get());

	CommandList->SetGraphicsRoot32BitConstants(0, 1, &GBufferATextureIndex, 0);
	CommandList->SetGraphicsRootDescriptorTable(1, GetTextureManager()->GetTexture2DGPUSRVForHeapStart());
	CommandList->SetGraphicsRootDescriptorTable(2, GetTextureManager()->GetTextureCubeGPUSRVForHeapStart());

	DrawRect(CommandList);

	// 3사분면->GBufferB
	D3D12_VIEWPORT ThirdQuadrantViewport = SecondQuadrantViewport;
	ThirdQuadrantViewport.TopLeftY = ThirdQuadrantViewport.Height;
	CommandList->RSSetViewports(1, &ThirdQuadrantViewport);

	CommandList->SetPipelineState(mPipelineStates["DrawRectPass"].Get());

	CommandList->SetGraphicsRootSignature(mRootSignatures["DrawRectPass"].Get());

	CommandList->SetGraphicsRoot32BitConstants(0, 1, &GBufferBTextureIndex, 0);
	CommandList->SetGraphicsRootDescriptorTable(1, GetTextureManager()->GetTexture2DGPUSRVForHeapStart());
	CommandList->SetGraphicsRootDescriptorTable(2, GetTextureManager()->GetTextureCubeGPUSRVForHeapStart());

	DrawRect(CommandList);

	// 4사분면->GBufferC
	D3D12_VIEWPORT ForthQuadrantViewport = SecondQuadrantViewport;
	ForthQuadrantViewport.TopLeftX = ForthQuadrantViewport.Width;
	ForthQuadrantViewport.TopLeftY = ForthQuadrantViewport.Height;
	CommandList->RSSetViewports(1, &ForthQuadrantViewport);

	CommandList->SetPipelineState(mPipelineStates["DrawRectPass"].Get());

	CommandList->SetGraphicsRootSignature(mRootSignatures["DrawRectPass"].Get());
	
	CommandList->SetGraphicsRoot32BitConstants(0, 1, &GBufferCTextureIndex, 0);
	CommandList->SetGraphicsRootDescriptorTable(1, GetTextureManager()->GetTexture2DGPUSRVForHeapStart());
	CommandList->SetGraphicsRootDescriptorTable(2, GetTextureManager()->GetTextureCubeGPUSRVForHeapStart());

	DrawRect(CommandList);

	

	FinishBackBuffer(CommandList);
}

void FDeferredShadingSceneRenderer::DrawGBuffers(ID3D12GraphicsCommandList* CommandList, FFrameResource* FrameResource)
{
	mGBuffers->TransitResourceBarrier(CommandList, "GBufferA", D3D12_RESOURCE_STATE_RENDER_TARGET);
	mGBuffers->TransitResourceBarrier(CommandList, "GBufferB", D3D12_RESOURCE_STATE_RENDER_TARGET);
	mGBuffers->TransitResourceBarrier(CommandList, "GBufferC", D3D12_RESOURCE_STATE_RENDER_TARGET);
	mGBuffers->TransitDepthStencilResourceBarrier(CommandList, D3D12_RESOURCE_STATE_DEPTH_WRITE);
	mGBuffers->ClearRenderTarget(CommandList, "GBufferA");
	mGBuffers->ClearRenderTarget(CommandList, "GBufferB");
	mGBuffers->ClearRenderTarget(CommandList, "GBufferC");
	mGBuffers->ClearDepthStencil(CommandList);

	D3D12_CPU_DESCRIPTOR_HANDLE GBufferARtv = mGBuffers->GetRTV("GBufferA", 0);
	D3D12_CPU_DESCRIPTOR_HANDLE GBufferDsv = mGBuffers->GetDSVHeap()->GetCPUDescriptorHandleForHeapStart();
	CommandList->OMSetRenderTargets(3, &GBufferARtv, true, &GBufferDsv);

	D3D12_VIEWPORT GBffuerViewport = mGBuffers->GetViewport();
	D3D12_RECT GBufferScissoreRect = mGBuffers->GetScissorRect();
	CommandList->RSSetViewports(1, &GBffuerViewport);
	CommandList->RSSetScissorRects(1, &GBufferScissoreRect);

	CommandList->SetPipelineState(mPipelineStates["GBufferPass"].Get());

	CommandList->SetGraphicsRootSignature(mRootSignatures["GBufferPass"].Get());

	ID3D12DescriptorHeap* DescriptorHeaps[] = { GetTextureManager()->GetSRVHeapPtr() };
	CommandList->SetDescriptorHeaps(_countof(DescriptorHeaps), DescriptorHeaps);

	ID3D12Resource* PassCB = FrameResource->GetPassCB()->Resource();
	CommandList->SetGraphicsRootConstantBufferView(0, PassCB->GetGPUVirtualAddress());
	// 1 : Mesh, 2 : Submesh 생략. DrawRenderItems에서 설정됨.
	ID3D12Resource* MaterialSB = FrameResource->GetMaterialSB()->Resource();
	CommandList->SetGraphicsRootShaderResourceView(3, MaterialSB->GetGPUVirtualAddress());
	CommandList->SetGraphicsRootDescriptorTable(4, GetTextureManager()->GetTexture2DGPUSRVForHeapStart());
	CommandList->SetGraphicsRootDescriptorTable(5, GetTextureManager()->GetTextureCubeGPUSRVForHeapStart());

	DrawRenderItems(FrameResource, CommandList, GetRenderItemManager()->GetRenderItems(ESM_DefaultLit, EBM_Opaque));

	mGBuffers->TransitResourceBarrier(CommandList, "GBufferA", D3D12_RESOURCE_STATE_GENERIC_READ);
	mGBuffers->TransitResourceBarrier(CommandList, "GBufferB", D3D12_RESOURCE_STATE_GENERIC_READ);
	mGBuffers->TransitResourceBarrier(CommandList, "GBufferC", D3D12_RESOURCE_STATE_GENERIC_READ);
	mGBuffers->TransitDepthStencilResourceBarrier(CommandList, D3D12_RESOURCE_STATE_DEPTH_READ);
}
