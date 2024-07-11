#pragma once
#include "Windows/Window.h"
#include "Common/Container.h"
#include "DirectX12/Direct3D12Util.h"
#include "UploadBuffer.h"
#include "Math/Math.h"

// Link necessary d3d12 libraries.
#pragma comment(lib,"d3dcompiler.lib")
#pragma comment(lib, "D3D12.lib")
#pragma comment(lib, "dxgi.lib")

struct Vertex
{
	DirectX::XMFLOAT3 Pos;
	DirectX::XMFLOAT4 Color;
};

struct ObjectConstants
{
	DirectX::XMFLOAT4X4 WorldViewProj = UMath::IdentityMatrix();

};

struct SubmeshGeometry
{
	UINT IndexCount = 0;
	UINT StartIndexLocation = 0;
	INT BaseVertexLocation = 0;

	// Bounding box of the geometry defined by this submesh. 
	// This is used in later chapters of the book.
	DirectX::BoundingBox Bounds;
};

struct MeshGeometry
{
	// Give it a name so we can look it up by name.
	std::string Name;

	// System memory copies.  Use Blobs because the vertex/index format can be generic.
	// It is up to the client to cast appropriately.  
	Microsoft::WRL::ComPtr<ID3DBlob> VertexBufferCPU = nullptr;
	Microsoft::WRL::ComPtr<ID3DBlob> IndexBufferCPU = nullptr;

	Microsoft::WRL::ComPtr<ID3D12Resource> VertexBufferGPU = nullptr;
	Microsoft::WRL::ComPtr<ID3D12Resource> IndexBufferGPU = nullptr;

	Microsoft::WRL::ComPtr<ID3D12Resource> VertexBufferUploader = nullptr;
	Microsoft::WRL::ComPtr<ID3D12Resource> IndexBufferUploader = nullptr;

	// Data about the buffers.
	UINT VertexByteStride = 0;
	UINT VertexBufferByteSize = 0;
	DXGI_FORMAT IndexFormat = DXGI_FORMAT_R16_UINT;
	UINT IndexBufferByteSize = 0;

	// A MeshGeometry may store multiple geometries in one vertex/index buffer.
	// Use this container to define the Submesh geometries so we can draw
	// the Submeshes individually.
	std::unordered_map<std::string, SubmeshGeometry> DrawArgs;

	D3D12_VERTEX_BUFFER_VIEW VertexBufferView()const
	{
		D3D12_VERTEX_BUFFER_VIEW vbv;
		vbv.BufferLocation = VertexBufferGPU->GetGPUVirtualAddress();
		vbv.StrideInBytes = VertexByteStride;
		vbv.SizeInBytes = VertexBufferByteSize;

		return vbv;
	}

	D3D12_INDEX_BUFFER_VIEW IndexBufferView()const
	{
		D3D12_INDEX_BUFFER_VIEW ibv;
		ibv.BufferLocation = IndexBufferGPU->GetGPUVirtualAddress();
		ibv.Format = IndexFormat;
		ibv.SizeInBytes = IndexBufferByteSize;

		return ibv;
	}

	// We can free this memory after we finish upload to the GPU.
	void DisposeUploaders()
	{
		VertexBufferUploader = nullptr;
		IndexBufferUploader = nullptr;
	}
};

Microsoft::WRL::ComPtr<ID3D12Resource> CreateDefaultBuffer(
	ID3D12Device* device,
	ID3D12GraphicsCommandList* cmdList,
	const void* initData,
	UINT64 byteSize,
	Microsoft::WRL::ComPtr<ID3D12Resource>& uploadBuffer)
{
	Microsoft::WRL::ComPtr<ID3D12Resource> defaultBuffer;

	// Create the actual default buffer resource.
	ThrowIfFailed(device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(byteSize),
		D3D12_RESOURCE_STATE_COMMON,
		nullptr,
		IID_PPV_ARGS(defaultBuffer.GetAddressOf())));

	// In order to copy CPU memory data into our default buffer, we need to create
	// an intermediate upload heap. 
	ThrowIfFailed(device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(byteSize),
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(uploadBuffer.GetAddressOf())));


	// Describe the data we want to copy into the default buffer.
	D3D12_SUBRESOURCE_DATA subResourceData = {};
	subResourceData.pData = initData;
	subResourceData.RowPitch = byteSize;
	subResourceData.SlicePitch = subResourceData.RowPitch;

	// Schedule to copy the data to the default buffer resource.  At a high level, the helper function UpdateSubresources
	// will copy the CPU memory into the intermediate upload heap.  Then, using ID3D12CommandList::CopySubresourceRegion,
	// the intermediate upload heap data will be copied to mBuffer.
	cmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(defaultBuffer.Get(),
		D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_COPY_DEST));
	UpdateSubresources<1>(cmdList, defaultBuffer.Get(), uploadBuffer.Get(), 0, 0, 1, &subResourceData);
	cmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(defaultBuffer.Get(),
		D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_GENERIC_READ));

	// Note: uploadBuffer has to be kept alive after the above function calls because
	// the command list has not been executed yet that performs the actual copy.
	// The caller can Release the uploadBuffer after it knows the copy has been executed.


	return defaultBuffer;
}


class FRenderer : public FWindow
{
	typedef FWindow Super;
public:
	FRenderer() {};
	~FRenderer() {};
	virtual bool Initialize(const WCHAR* Title = L"DefaultTitle",
		int CmdShow = 10, UINT Width = 1920, UINT Height = 1080,
		DWORD Style = WS_OVERLAPPED | WS_SYSMENU) override;

protected:
	void Resize();
	void FlushCommandQueue();
	virtual void Event() override;
	virtual void Render();
	inline ID3D12Resource* CurrentBackBuffer();
	D3D12_CPU_DESCRIPTOR_HANDLE GetCurrentBackBufferView()const;
	D3D12_CPU_DESCRIPTOR_HANDLE GetDepthStencilView()const;

	Microsoft::WRL::ComPtr<IDXGIFactory4> DXGIFactory = nullptr;
	Microsoft::WRL::ComPtr<ID3D12Device> D3D12Device = nullptr;
	Microsoft::WRL::ComPtr<ID3D12Fence> D3D12Fence = nullptr;
	Microsoft::WRL::ComPtr<ID3D12CommandQueue> D3D12CommandQueue = nullptr;
	Microsoft::WRL::ComPtr<ID3D12CommandAllocator> D3D12CommandListAllocator = nullptr;
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> D3D12GraphicsCommandList = nullptr;
	Microsoft::WRL::ComPtr<IDXGISwapChain> DXGISwapChain = nullptr;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> D3D12RTVHeap = nullptr;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> D3D12DSVHeap = nullptr;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> D3D12CBVHeap = nullptr;
	static const int SwapChainBufferCount = 2;
	Microsoft::WRL::ComPtr<ID3D12Resource> D3D12SwapChainBuffer[SwapChainBufferCount];
	Microsoft::WRL::ComPtr<ID3D12Resource> D3D12DepthStencilBuffer;
	Microsoft::WRL::ComPtr<ID3D12RootSignature> mRootSignature;
	Microsoft::WRL::ComPtr<ID3DBlob> mvsByteCode;
	Microsoft::WRL::ComPtr<ID3DBlob> mpsByteCode;
	std::vector<D3D12_INPUT_ELEMENT_DESC> mInputLayout;
	std::unique_ptr<FUploadBuffer<ObjectConstants>> mObjectCB;
	std::unique_ptr<MeshGeometry> mBoxGeo = nullptr;

	UINT64 CurrentFence = 0;
	UINT RTVDescriptorSize = 0;		// RenderTargetView Descriptor Size
	UINT DSVDescriptorSize = 0;		// DepthStencilView Descriptor Size
	UINT CBVSRVUAVDescriptorSize = 0;	// ConstantBufferView, ShaderResourceView, UnorderedAccessView Descriptor Size
	UINT MSAAQuality = 0;
	DXGI_FORMAT DXGIBackBufferFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
	DXGI_FORMAT DXGIDepthStencilFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
	D3D12_VIEWPORT D3D12Viewport = D3D12_VIEWPORT();
	D3D12_RECT D3D12ScissorRect = D3D12_RECT();

	int CurrBackBuffer = 0;
	bool MSAAState = false;    // 4X MSAA enabled

private:
	void LogAdapters();
	void LogAdapterOutputs(IDXGIAdapter* Adapter);
	void LogOutputDisplayModes(IDXGIOutput* Output, DXGI_FORMAT Format);
	void CreateCommandObjects();
	void CreateSwapChain();
	virtual void CreateRTVAndDSVDescriptorHeaps();

	inline D3D12_RESOURCE_BARRIER CreateResoureceBarrierTransition(
		ID3D12Resource* Resource,
		D3D12_RESOURCE_STATES StateBefore,
		D3D12_RESOURCE_STATES StateAfter,
		UINT Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES,
		D3D12_RESOURCE_BARRIER_FLAGS Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE
	);
};

