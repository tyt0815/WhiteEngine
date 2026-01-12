#include "ShapeDrawer.h"
#include "GeometryGenerator.h"
#include "MeshGeometry.h"
#include "DirectX/DXResourceManager.h"

FShapeDrawer::FShapeDrawer()
{
	GetDXResourceManagerPtr()->ExecuteAndFlushCommand(&FShapeDrawer::BuildVertexBuffers, this);
    BuildRootSignature();
    ComplieShaders();
    BuildInputLayouts();
    BuildPipelineStates();
}

FShapeDrawer::~FShapeDrawer()
{
}

void FShapeDrawer::DrawLine3D(ID3D12GraphicsCommandList* CommandList, XMFLOAT3 Start, XMFLOAT3 End, XMFLOAT4 Color)
{
}

void FShapeDrawer::BuildRootSignature()
{
	// Tip: 자주 사용되는 것일수록 작은 인덱스에 보관하는게 퍼포먼스가 좋음
	constexpr UINT ROOT_PARAMETERs_NUM = 1;
	CD3DX12_ROOT_PARAMETER RootParameter[ROOT_PARAMETERs_NUM];
	RootParameter[0].InitAsConstantBufferView(0, 0);	// CB

	FDXUtility::BuildRootSignature(RootParameter, ROOT_PARAMETERs_NUM, mRootSignature.GetAddressOf());
}

void FShapeDrawer::ComplieShaders()
{
	mShaders["DrawLinePassVertexShader"] = FDXUtility::CompileShader(
		std::wstring(PROJECT_DIR_W) + L"\\Shaders\\DrawLine3DPass.hlsl",
		nullptr,
		"MainVS",
		"vs_5_1"
	);
	mShaders["DrawLinePassPixelShader"] = FDXUtility::CompileShader(
		std::wstring(PROJECT_DIR_W) + L"\\Shaders\\DrawLine3DPass.hlsl",
		nullptr,
		"MainPS",
		"ps_5_1"
	);	
}

void FShapeDrawer::BuildInputLayouts()
{
	mInputLayouts["DrawLinePass"] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
	};

	
}

void FShapeDrawer::BuildPipelineStates()
{
	D3D12_GRAPHICS_PIPELINE_STATE_DESC DrawLinePassPipelineState;
	ZeroMemory(&DrawLinePassPipelineState, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
	DrawLinePassPipelineState.InputLayout = { mInputLayouts["DrawLinePass"].data(), (UINT)mInputLayouts["DrawLinePass"].size() };
	DrawLinePassPipelineState.pRootSignature = mRootSignature.Get();
	DrawLinePassPipelineState.VS =
	{
		reinterpret_cast<BYTE*>(mShaders["DrawLinePassVertexShader"]->GetBufferPointer()),
		mShaders["DrawLinePassVertexShader"]->GetBufferSize()
	};
	DrawLinePassPipelineState.PS =
	{
		reinterpret_cast<BYTE*>(mShaders["DrawLinePassPixelShader"]->GetBufferPointer()),
		mShaders["DrawLinePassPixelShader"]->GetBufferSize()
	};
	DrawLinePassPipelineState.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	DrawLinePassPipelineState.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	DrawLinePassPipelineState.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	DrawLinePassPipelineState.SampleMask = UINT_MAX;
	DrawLinePassPipelineState.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE;
	DrawLinePassPipelineState.NumRenderTargets = 1;
	DrawLinePassPipelineState.RTVFormats[0] = GetDXResourceManagerPtr()->GetBackbufferFormat();
	DrawLinePassPipelineState.SampleDesc.Count = 1;
	DrawLinePassPipelineState.SampleDesc.Quality = 0;
	DrawLinePassPipelineState.DSVFormat = GetDXResourceManagerPtr()->GetDepthStencilFormat();
	THROW_IF_FAILED(
		GetDXResourceManagerPtr()->GetDevicePtr()->CreateGraphicsPipelineState(
			&DrawLinePassPipelineState,
			IID_PPV_ARGS(mPipelineStates["DrawLinePass"].GetAddressOf())
		)
	);
}

void FShapeDrawer::BuildVertexBuffers(ID3D12Device* Device, ID3D12GraphicsCommandList* CommandList)
{
	std::vector<XMFLOAT3> Vertices(1);
	const UINT VBByteSize = static_cast<UINT>(Vertices.size() * sizeof(XMFLOAT3));

	THROW_IF_FAILED(D3DCreateBlob(VBByteSize, VertexBufferCPU.GetAddressOf()));
	CopyMemory(VertexBufferCPU->GetBufferPointer(), Vertices.data(), VBByteSize);

	VertexBufferGPU = FDXUtility::CreateDefaultBuffer(
		Device,
		CommandList,
		Vertices.data(),
		VBByteSize,
		VertexBufferUploader
	);
}


std::vector<D3D12_INPUT_ELEMENT_DESC> GetDrawingSphereInputLayouts()
{
    return {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
    };
}

void DrawSphere(ID3D12GraphicsCommandList* CommandList)
{
    FMeshGeometry* Sphere = GetMeshGeometryManager()->GetMeshGeometry("Sphere");
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

std::vector<D3D12_INPUT_ELEMENT_DESC> GetDrawingRectInputLayouts()
{
    return {
        { "POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "TEXC", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 8, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
    };
}

void DrawRect(ID3D12GraphicsCommandList* CommandList)
{
    FMeshGeometry* Rectangle = GetMeshGeometryManager()->GetMeshGeometry("Rectangle");
    D3D12_VERTEX_BUFFER_VIEW VertexBufferView = Rectangle->VertexBufferView();
    D3D12_INDEX_BUFFER_VIEW IndexBufferView = Rectangle->IndexBufferView();

    CommandList->IASetVertexBuffers(0, 1, &VertexBufferView);
    CommandList->IASetIndexBuffer(&IndexBufferView);
    CommandList->IASetPrimitiveTopology(Rectangle->PrimitiveType);

    CommandList->DrawIndexedInstanced(
        Rectangle->DrawArgs[0].IndexCount,
        1,
        Rectangle->DrawArgs[0].StartIndexLocation,
        Rectangle->DrawArgs[0].BaseVertexLocation,
        0
    );
}

void DrawMeshGeometry(ID3D12GraphicsCommandList* CommandList, const FMeshDrawInfo& MeshDrawInfo)
{
    CommandList->SetGraphicsRootConstantBufferView(1, MeshDrawInfo.ObjectConstantBufferAddress);
    CommandList->SetGraphicsRootConstantBufferView(2, MeshDrawInfo.SubmeshConstantBufferAddress);

    CommandList->IASetVertexBuffers(0, 1, &MeshDrawInfo.VertexBufferView);
    CommandList->IASetIndexBuffer(&MeshDrawInfo.IndexBufferView);
    CommandList->IASetPrimitiveTopology(MeshDrawInfo.PrimitiveTopology);

    CommandList->DrawIndexedInstanced(
        MeshDrawInfo.IndexCount,
        1,
        MeshDrawInfo.StartIndexLocation,
        MeshDrawInfo.BaseVertexLocation,
        0
    );
}

