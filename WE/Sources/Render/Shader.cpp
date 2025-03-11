#include "Shader.h"
#include "FrameResource.h"
#include "DirectX/DXResourceManager.h"

FShaderManager::FShaderManager()
{
	mPiplineStates.resize(ESM_None);
	for (int i = 0; i < mPiplineStates.size(); ++i)
	{
		mPiplineStates[i].resize(EBM_None);
	}
	BuildShaderAndInputLayout();
	BuildPipelineStateObject();
}

FShaderManager::~FShaderManager()
{

}

void FShaderManager::BuildShaderAndInputLayout()
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

	mShaders["ForwardLitVertexShader"] = FDXUtility::CompileShader(
		L"Shaders\\ForwardLitVertexShader.sf",
		nullptr,
		"MainVS",
		"vs_5_1"
	);
	mShaders["ForwardLitPixelShader"] = FDXUtility::CompileShader(
		L"Shaders\\ForwardLitPixelShader.sf",
		Defines,
		"MainPS",
		"ps_5_1"
	);

	mShaders["AlphTestPixelShader"] = FDXUtility::CompileShader(
		L"Shaders\\ForwardLitPixelShader.sf",
		AlphaTestDefine,
		"MainPS",
		"ps_5_1"
	);

	mShaders["BillboardVertexShader"] = FDXUtility::CompileShader(
		L"Shaders\\BillboardVertexShader.sf",
		nullptr,
		"MainVS",
		"vs_5_1"
	);

	mShaders["BillboardGeometryShader"] = FDXUtility::CompileShader(
		L"Shaders\\BillboardGeometryShader.sf",
		nullptr,
		"MainGS",
		"gs_5_1"
	);

	mShaders["BillboardPixelShader"] = FDXUtility::CompileShader(
		L"Shaders\\BillboardPixelShader.sf",
		AlphaTestDefine,
		"MainPS",
		"ps_5_1"
	);

	mInputLayouts["Lit"] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
	};

	mInputLayouts["Billboard"] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "SIZE", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
	};
}

void FShaderManager::BuildPipelineStateObject()
{
	FDXResourceManager* DeviceManager = FDXResourceManager::GetInstance();
	ID3D12Device* Device = DeviceManager->GetDevicePtr();
	D3D12_GRAPHICS_PIPELINE_STATE_DESC ForwardLitPipelineStateDesc;
	//
	// PSO for opaque objects.
	//
	ZeroMemory(&ForwardLitPipelineStateDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
	ForwardLitPipelineStateDesc.InputLayout = { mInputLayouts["Lit"].data(), (UINT)mInputLayouts["Lit"].size() };
	ForwardLitPipelineStateDesc.pRootSignature = GetFrameResourceManager()->GetRootSignaturePtr();
	ForwardLitPipelineStateDesc.VS =
	{
		reinterpret_cast<BYTE*>(mShaders["ForwardLitVertexShader"]->GetBufferPointer()),
		mShaders["ForwardLitVertexShader"]->GetBufferSize()
	};
	ForwardLitPipelineStateDesc.PS =
	{
		reinterpret_cast<BYTE*>(mShaders["ForwardLitPixelShader"]->GetBufferPointer()),
		mShaders["ForwardLitPixelShader"]->GetBufferSize()
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
	THROW_IF_FAILED(
		Device->CreateGraphicsPipelineState(
			&ForwardLitPipelineStateDesc,
			IID_PPV_ARGS(mPiplineStates[ESM_DefaultLit][EBM_Opaque].GetAddressOf())
		)
	);


	//
	// PSO for opaque wireframe objects.
	//

	{
		D3D12_GRAPHICS_PIPELINE_STATE_DESC WireFramePipelineStateDesc = ForwardLitPipelineStateDesc;
		WireFramePipelineStateDesc.RasterizerState.FillMode = D3D12_FILL_MODE_WIREFRAME;
		THROW_IF_FAILED(
			Device->CreateGraphicsPipelineState(
				&WireFramePipelineStateDesc, IID_PPV_ARGS(mWireFramePipelineState.GetAddressOf())
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
		THROW_IF_FAILED(
			Device->CreateGraphicsPipelineState(
				&TransparencyPipelineStateDesc,
				IID_PPV_ARGS(mPiplineStates[ESM_DefaultLit][EBM_Transparency].GetAddressOf())
			)
		);
	}

	{
		D3D12_GRAPHICS_PIPELINE_STATE_DESC AlphaTestPipelineStateDesc = ForwardLitPipelineStateDesc;
		AlphaTestPipelineStateDesc.PS =
		{
			reinterpret_cast<BYTE*>(mShaders["AlphTestPixelShader"]->GetBufferPointer()),
			mShaders["AlphTestPixelShader"]->GetBufferSize()
		};

		AlphaTestPipelineStateDesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
		THROW_IF_FAILED(
			Device->CreateGraphicsPipelineState(
				&AlphaTestPipelineStateDesc,
				IID_PPV_ARGS(mPiplineStates[ESM_DefaultLit][EBM_AlphaTest].GetAddressOf())
			)
		);
	}

	// Billboard
	{
		D3D12_GRAPHICS_PIPELINE_STATE_DESC BillboardPipelineStateDesc = ForwardLitPipelineStateDesc;
		BillboardPipelineStateDesc.VS =
		{
			reinterpret_cast<BYTE*>(mShaders["BillboardVertexShader"]->GetBufferPointer()),
			mShaders["BillboardVertexShader"]->GetBufferSize()
		};
		BillboardPipelineStateDesc.GS =
		{
			reinterpret_cast<BYTE*>(mShaders["BillboardGeometryShader"]->GetBufferPointer()),
			mShaders["BillboardGeometryShader"]->GetBufferSize()
		};
		BillboardPipelineStateDesc.PS =
		{
			reinterpret_cast<BYTE*>(mShaders["BillboardPixelShader"]->GetBufferPointer()),
			mShaders["BillboardPixelShader"]->GetBufferSize()
		};
		BillboardPipelineStateDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT;
		BillboardPipelineStateDesc.InputLayout = { mInputLayouts["Billboard"].data(), (UINT)mInputLayouts["Billboard"].size() };
		BillboardPipelineStateDesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;

		THROW_IF_FAILED(
			Device->CreateGraphicsPipelineState(
				&BillboardPipelineStateDesc,
				IID_PPV_ARGS(mBillboardPipelineState.GetAddressOf())
			)
		);
	}
}
