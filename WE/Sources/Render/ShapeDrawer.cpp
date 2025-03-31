#include "ShapeDrawer.h"
#include "GeometryGenerator.h"
#include "MeshGeometry.h"
#include "DirectX/DXResourceManager.h"

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
