#include "CubeSkyRenderer.h"
#include "Texture.h"
#include "FrameResource.h"
#include "DirectX/DXResourceManager.h"
#include "MeshGeometry.h"

FCubeSkyRenderer::FCubeSkyRenderer()
{
	BuildShaderAndInputLayout();
	BuildPipelineStateObject();
}

FCubeSkyRenderer::~FCubeSkyRenderer()
{
}

void FCubeSkyRenderer::Render(ID3D12GraphicsCommandList* CommandList)
{
	CommandList->SetPipelineState(mPipelineState.Get());

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
}