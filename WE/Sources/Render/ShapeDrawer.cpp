#include "ShapeDrawer.h"
#include "GeometryGenerator.h"
#include "MeshGeometry.h"
#include "DirectX/DXResourceManager.h"

FShapeDrawer::FShapeDrawer()
{
    GetDXResourceManagerPtr()->ExecuteAndFlushCommand(&FShapeDrawer::BuildBuffers, this);
}

FShapeDrawer::~FShapeDrawer()
{
}

std::vector<D3D12_INPUT_ELEMENT_DESC> FShapeDrawer::GetDrawingSphereInputLayouts()
{
    return {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
    };
}

void FShapeDrawer::DrawSphere(ID3D12GraphicsCommandList* CommandList)
{
    D3D12_VERTEX_BUFFER_VIEW VertexBufferView = mSphere->VertexBufferView();
    D3D12_INDEX_BUFFER_VIEW IndexBufferView = mSphere->IndexBufferView();

    CommandList->IASetVertexBuffers(0, 1, &VertexBufferView);
    CommandList->IASetIndexBuffer(&IndexBufferView);
    CommandList->IASetPrimitiveTopology(mSphere->PrimitiveType);

    CommandList->DrawIndexedInstanced(
        mSphere->DrawArgs[0].IndexCount,
        1,
        mSphere->DrawArgs[0].StartIndexLocation,
        mSphere->DrawArgs[0].BaseVertexLocation,
        0
    );
}
void FShapeDrawer::BuildBuffers(ID3D12Device* Device, ID3D12GraphicsCommandList* CommandList)
{
    BuildSphereBuffer(Device, CommandList);
}

void FShapeDrawer::BuildSphereBuffer(ID3D12Device* Device, ID3D12GraphicsCommandList* CommandList)
{
    //FSubmeshGeometry Submesh;
    //Submesh.IndexCount = (UINT)MeshData.Indices32.size();
    //Submesh.StartIndexLocation = 0;
    //Submesh.BaseVertexLocation = 0;
    //std::vector<FVertex> Vertices(MeshData.Vertices.size());
    //for (size_t i = 0; i < MeshData.Vertices.size(); ++i)
    //{
    //    Vertices[i].Pos = MeshData.Vertices[i].Position;
    //    Vertices[i].Normal = MeshData.Vertices[i].Normal;
    //    Vertices[i].TexC = MeshData.Vertices[i].TexC;
    //}
    //std::vector<std::uint32_t> Indices;
    //Indices.insert(Indices.end(), std::begin(MeshData.Indices32), std::end(MeshData.Indices32));
    //BuildMeshGeometryU32(Name, Vertices, Indices, std::vector<FSubmeshGeometry>(1, Submesh), D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST, Device, CommandList);

    struct FPosVertex
    {
        XMFLOAT3 Position;
    };
    FGeometryGenerator GeoGen;
    FGeometryGenerator::MeshData MeshData = GeoGen.CreateSphere(0.5f, 20, 20);
    std::vector<FPosVertex> Vertices(MeshData.Vertices.size());
    for (std::uint32_t i = 0; i < Vertices.size(); ++i)
    {
        Vertices[i].Position = MeshData.Vertices[i].Position;
    }
    std::vector<std::uint32_t> Indices;
    Indices.insert(Indices.end(), std::begin(MeshData.Indices32), std::end(MeshData.Indices32));
    FSubmeshGeometry Submesh;
    Submesh.IndexCount = (UINT)MeshData.Indices32.size();
    Submesh.StartIndexLocation = 0;
    Submesh.BaseVertexLocation = 0;
    std::vector<FSubmeshGeometry> Submeshs(1, Submesh);
    GetMeshGeometryManager()->BuildMeshGeometryU32(
        mSphereName,
        Vertices,
        Indices,
        Submeshs,
        D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST,
        Device,
        CommandList
    );
    mSphere = GetMeshGeometryManager()->GetMeshGeometry(mSphereName);
}
