#include "ForwardShadingSceneRenderer.h"

#include <DirectXColors.h>

#include "MeshGeometry.h"
#include "Texture.h"
#include "Material.h"
#include "DirectX/DXResourceManager.h"
#include "DirectX/DXException.h"
#include "DirectX/DXUtility.h"
#include "DirectX/SRVHeap.h"
#include "RenderItemManager.h"

void FForwardShadingSceneRenderer::Initialize(ID3D12Device* Device)
{
	Super::Initialize(Device);
	// mSkyCubeMapRenderer = std::make_unique<FCubeSkyRenderer>(std::string("Snow"));
}

void FForwardShadingSceneRenderer::Render(
	ID3D12GraphicsCommandList* CommandList,
	FFrameResourceBase* FrameResource
)
{
	ReadyBackBuffer(CommandList);

	D3D12_CPU_DESCRIPTOR_HANDLE RenderTargetView = GetDXResourceManagerPtr()->GetCurrentBackBufferView();
	D3D12_CPU_DESCRIPTOR_HANDLE DepthStencilView = GetDXResourceManagerPtr()->GetDepthStencilView();
	CommandList->OMSetRenderTargets(1, &RenderTargetView, true, &DepthStencilView);

	CommandList->SetGraphicsRootSignature(mRootSignatures["ForwardShadingPass"].Get());
	FTextureManager* TexManager = GetTextureManager();
	FSRVHeap* SRVHeap = GetSRVHeap();
	ID3D12DescriptorHeap* descriptorHeaps[] = { SRVHeap->Get()};
	CommandList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);

	// Pass Constantbuffer
	ID3D12Resource* PassConstantBuffer = FrameResource->GetPassCB()->Resource();
	D3D12_GPU_VIRTUAL_ADDRESS PassConstantBufferAdress = PassConstantBuffer->GetGPUVirtualAddress();
	CommandList->SetGraphicsRootConstantBufferView(0, PassConstantBufferAdress);
	ID3D12Resource* MaterialSB = FrameResource->GetMaterialSB()->Resource();
	ID3D12Resource* DirLightSB = FrameResource->GetDirectionalLightSB()->Resource();
	CommandList->SetGraphicsRootShaderResourceView(3, MaterialSB->GetGPUVirtualAddress());
	CommandList->SetGraphicsRootShaderResourceView(4, DirLightSB->GetGPUVirtualAddress());
	CommandList->SetGraphicsRootDescriptorTable(5, SRVHeap->GetTexture2DSRVStart());
	CommandList->SetGraphicsRootDescriptorTable(6, SRVHeap->GetTextureCubeSRVStart());
	if (bWireFrame)
	{
		// TODO
	}
	else
	{
		for (size_t i = 0; i < ESM_None; ++i)
		{
			EShadingModel ShadingModel = static_cast<EShadingModel>(i);
			for (size_t j = 0; j < EBM_None; ++j)
			{
				EBlendMode BlendMode = static_cast<EBlendMode>(j);
				CommandList->SetPipelineState(mPipelineStates["ForwardShadingPass_Opaque"].Get());
				DrawRenderItems(FrameResource, CommandList, GetRenderItemManager()->GetRenderItems(ShadingModel, BlendMode));
			}
		}
	}
	
	// mSkyCubeMapRenderer->Render(CommandList);

	FinishBackBuffer(CommandList);
}

void FForwardShadingSceneRenderer::BuildShadersAndInputLayouts()
{
	Super::BuildShadersAndInputLayouts();
	D3D_SHADER_MACRO Defines[] = {
		//{"FOG", "1"},
		{"FORWARDSHADING", "1"},
		{NULL, NULL}
	};

	mShaders["ForwardShadingPassVertexShader"] = FDXUtility::CompileShader(
		L"Shaders\\ForwardShadingPassVertexShader.sf",
		nullptr,
		"MainVS",
		"vs_5_1"
	);
	mShaders["ForwardShadingPassPixelShader"] = FDXUtility::CompileShader(
		L"Shaders\\ForwardShadingPassPixelShader.sf",
		Defines,
		"MainPS",
		"ps_5_1"
	);
}

void FForwardShadingSceneRenderer::BuildPipelineStates(ID3D12Device* Device)
{
	Super::BuildPipelineStates(Device);

	D3D12_GRAPHICS_PIPELINE_STATE_DESC ForwardLitPipelineStateDesc;
	ZeroMemory(&ForwardLitPipelineStateDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
	ForwardLitPipelineStateDesc.InputLayout = { mInputLayouts["MeshGeometryPass"].data(), (UINT)mInputLayouts["MeshGeometryPass"].size() };
	ForwardLitPipelineStateDesc.pRootSignature = mRootSignatures["ForwardShadingPass"].Get();
	ForwardLitPipelineStateDesc.VS =
	{
		reinterpret_cast<BYTE*>(mShaders["ForwardShadingPassVertexShader"]->GetBufferPointer()),
		mShaders["ForwardShadingPassVertexShader"]->GetBufferSize()
	};
	ForwardLitPipelineStateDesc.PS =
	{
		reinterpret_cast<BYTE*>(mShaders["ForwardShadingPassPixelShader"]->GetBufferPointer()),
		mShaders["ForwardShadingPassPixelShader"]->GetBufferSize()
	};
	ForwardLitPipelineStateDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	ForwardLitPipelineStateDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	ForwardLitPipelineStateDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	ForwardLitPipelineStateDesc.SampleMask = UINT_MAX;
	ForwardLitPipelineStateDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	ForwardLitPipelineStateDesc.NumRenderTargets = 1;
	ForwardLitPipelineStateDesc.RTVFormats[0] = GetDXResourceManagerPtr()->GetBackbufferFormat();
	ForwardLitPipelineStateDesc.SampleDesc.Count = GetDXResourceManagerPtr()->IsMSAAOn() ? 4 : 1;
	ForwardLitPipelineStateDesc.SampleDesc.Quality = GetDXResourceManagerPtr()->IsMSAAOn() ? (GetDXResourceManagerPtr()->GetMSAAQuality_4x() - 1) : 0;
	ForwardLitPipelineStateDesc.DSVFormat = GetDXResourceManagerPtr()->GetDepthStencilFormat();
	THROW_IF_FAILED(
		Device->CreateGraphicsPipelineState(
			&ForwardLitPipelineStateDesc,
			IID_PPV_ARGS(mPipelineStates["ForwardShadingPass_Opaque"].GetAddressOf())
		)
	);

	{
		D3D12_GRAPHICS_PIPELINE_STATE_DESC WireFramePipelineStateDesc = ForwardLitPipelineStateDesc;
		WireFramePipelineStateDesc.RasterizerState.FillMode = D3D12_FILL_MODE_WIREFRAME;
		THROW_IF_FAILED(
			Device->CreateGraphicsPipelineState(
				&WireFramePipelineStateDesc, IID_PPV_ARGS(mPipelineStates["WireFramePass"].GetAddressOf())
			)
		);
	}
}

void FForwardShadingSceneRenderer::CreateFrameResources(ID3D12Device* Device)
{
	CreateFrameResources_Internal<FFSFrameResource>(Device);
}

void FForwardShadingSceneRenderer::BuildRootSignature()
{
	Super::BuildRootSignature();
	D3D12_DESCRIPTOR_RANGE TextureTable = GetTextureManager()->GetTexture2DDescriptorRange();
	D3D12_DESCRIPTOR_RANGE CubeTextureTable = GetTextureManager()->GetTextureCubeDescriptorRange();

	// Tip: 자주 사용되는 것일수록 작은 인덱스에 보관하는게 퍼포먼스가 좋음
	constexpr UINT ROOT_PARAMETERs_NUM = 7;
	CD3DX12_ROOT_PARAMETER RootParameter[ROOT_PARAMETERs_NUM];
	RootParameter[0].InitAsConstantBufferView(0);	// PassCB
	RootParameter[1].InitAsConstantBufferView(1);	// MeshCB
	RootParameter[2].InitAsConstantBufferView(2);	// SubmeshCB
	RootParameter[3].InitAsShaderResourceView(0, 2);	// MaterialSB
	RootParameter[4].InitAsShaderResourceView(0, 3);	// DirLightSB
	RootParameter[5].InitAsDescriptorTable(1, &TextureTable, D3D12_SHADER_VISIBILITY_PIXEL);	// TextureTable
	RootParameter[6].InitAsDescriptorTable(1, &CubeTextureTable, D3D12_SHADER_VISIBILITY_PIXEL);	// CubeTextureTable

	FDXUtility::BuildRootSignature(RootParameter, ROOT_PARAMETERs_NUM, mRootSignatures["ForwardShadingPass"].GetAddressOf());
}

FForwardShadingSceneRenderer::FFSFrameResource::FFSFrameResource(ID3D12Device* Device) :
	FFrameResourceBase(Device)
{
}
