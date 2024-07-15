#include <array>
#include "MeshGeometry.h"

FMeshGeometry BuildBoxMeshGeometry(ID3D12Device* Device, ID3D12GraphicsCommandList* CommandList)
{
	static std::array<FVertex, 8> Vertices =
	{
		FVertex({ XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT4(Colors::White) }),
		FVertex({ XMFLOAT3(-1.0f, +1.0f, -1.0f), XMFLOAT4(Colors::Black) }),
		FVertex({ XMFLOAT3(+1.0f, +1.0f, -1.0f), XMFLOAT4(Colors::Red) }),
		FVertex({ XMFLOAT3(+1.0f, -1.0f, -1.0f), XMFLOAT4(Colors::Green) }),
		FVertex({ XMFLOAT3(-1.0f, -1.0f, +1.0f), XMFLOAT4(Colors::Blue) }),
		FVertex({ XMFLOAT3(-1.0f, +1.0f, +1.0f), XMFLOAT4(Colors::Yellow) }),
		FVertex({ XMFLOAT3(+1.0f, +1.0f, +1.0f), XMFLOAT4(Colors::Cyan) }),
		FVertex({ XMFLOAT3(+1.0f, -1.0f, +1.0f), XMFLOAT4(Colors::Magenta) })
	};

	static std::array<std::uint16_t, 36> Indices =
	{
		// front face
		0, 1, 2,
		0, 2, 3,

		// back face
		4, 6, 5,
		4, 7, 6,

		// left face
		4, 5, 1,
		4, 1, 0,

		// right face
		3, 2, 6,
		3, 6, 7,

		// top face
		1, 5, 6,
		1, 6, 2,

		// bottom face
		4, 0, 3,
		4, 3, 7
	};

	const UINT VertexBufferByteSize = (UINT)Vertices.size() * sizeof(FVertex);
	const UINT IndexBufferByteSize = (UINT)Indices.size() * sizeof(std::uint16_t);

	FMeshGeometry BoxGeometry;
	BoxGeometry.Name = "boxGeo";

	THROWIFFAILED(D3DCreateBlob(VertexBufferByteSize, &BoxGeometry.VertexBufferCPU));
	CopyMemory(BoxGeometry.VertexBufferCPU->GetBufferPointer(), Vertices.data(), VertexBufferByteSize);

	THROWIFFAILED(D3DCreateBlob(IndexBufferByteSize, &BoxGeometry.IndexBufferCPU));
	CopyMemory(BoxGeometry.IndexBufferCPU->GetBufferPointer(), Indices.data(), IndexBufferByteSize);

	BoxGeometry.VertexBufferGPU = UDXUtility::CreateDefaultBuffer(
		Device,
		CommandList,
		Vertices.data(),
		VertexBufferByteSize,
		BoxGeometry.VertexBufferUploader
	);

	BoxGeometry.IndexBufferGPU = UDXUtility::CreateDefaultBuffer(
		Device,
		CommandList,
		Indices.data(),
		IndexBufferByteSize,
		BoxGeometry.IndexBufferUploader
	);

	BoxGeometry.VertexByteStride = sizeof(FVertex);
	BoxGeometry.VertexBufferByteSize = VertexBufferByteSize;
	BoxGeometry.IndexFormat = DXGI_FORMAT_R16_UINT;
	BoxGeometry.IndexBufferByteSize = IndexBufferByteSize;

	SubmeshGeometry Submesh;
	Submesh.IndexCount = (UINT)Indices.size();
	Submesh.StartIndexLocation = 0;
	Submesh.BaseVertexLocation = 0;

	BoxGeometry.DrawArgs["box"] = Submesh;
	return BoxGeometry;
}
