#include "RenderTarget.h"
#include "DirectX/DXException.h"
#include "DirectX/DXResourceManager.h"
#include "Texture.h"

float FRenderTarget::sClearColor[4] = { 0.0f, 0.0f, 0.0f, 1.0f };

FRenderTarget::FRenderTarget(
	std::vector<std::string> Names,
	UINT Width,
	UINT Height,
	UINT MipLevels,
	DXGI_FORMAT Format,
	DXGI_FORMAT DepthStencilFormat
) :
	mWidth(Width),
	mHeight(Height),
	mMipLevels(MipLevels),
	mFormat(Format),
	mDepthStencilFormat(DepthStencilFormat),
	mViewport({ 0.0f, 0.0f, (float)Width, (float)Height, 0.0f, 1.0f }),
	mScissorRect({ 0, 0, (int)Width, (int)Height })
{
	Initialize(Names);
}

FRenderTarget::~FRenderTarget()
{
}

void FRenderTarget::TransitResourceBarrier(
	ID3D12GraphicsCommandList* CommandList,
	std::string Name,
	D3D12_RESOURCE_STATES ResourceState
)
{
	FResourceInfo& ResourceInfo = mResourceMap[Name];
	if (ResourceState != ResourceInfo.ResourceState)
	{
		D3D12_RESOURCE_BARRIER ResourceBarrier = CD3DX12_RESOURCE_BARRIER::Transition(
			ResourceInfo.Texture->Resource.Get(),
			ResourceInfo.ResourceState,
			ResourceState
		);
		CommandList->ResourceBarrier(1, &ResourceBarrier);
		ResourceInfo.ResourceState = ResourceState;
	}
}

void FRenderTarget::TransitDepthStencilResourceBarrier(
	ID3D12GraphicsCommandList* CommandList,
	D3D12_RESOURCE_STATES ResourceState
)
{
	if (ResourceState != mDepthStencilState)
	{
		D3D12_RESOURCE_BARRIER ResourceBarrier = CD3DX12_RESOURCE_BARRIER::Transition(
			mDepthStencilResource.Get(),
			mDepthStencilState,
			ResourceState
		);
		CommandList->ResourceBarrier(1, &ResourceBarrier);
		mDepthStencilState = ResourceState;
	}
}

void FRenderTarget::ClearRenderTarget(ID3D12GraphicsCommandList* CommandList, std::string Name, int MipLevel)
{
	CommandList->ClearRenderTargetView(
		GetRTV(Name, MipLevel),
		sClearColor,
		0,
		nullptr
	);
}

void FRenderTarget::ClearDepthStencil(ID3D12GraphicsCommandList* CommandList)
{
	CommandList->ClearDepthStencilView(
		GetDSV(),
		D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL,
		1.0f,
		0,
		0,
		nullptr
	);
}

D3D12_CPU_DESCRIPTOR_HANDLE FRenderTarget::GetRTV(std::string Name, int MipLevel)
{
	return CD3DX12_CPU_DESCRIPTOR_HANDLE(
		mRTVHeap->GetCPUDescriptorHandleForHeapStart(),
		(int)mResourceMap[Name].Index + (int)mResourceMap.size() * MipLevel,
		GetDXResourceManagerPtr()->GetRTVDescriptorSize()
	);
}

D3D12_CPU_DESCRIPTOR_HANDLE FRenderTarget::GetCPUDescriptorHeap(std::string Name)
{
	return GetTextureManager()->GetTexture2DCPUDescriptorHandle(mResourceMap[Name].Texture->SRVHeapIndex);
}

D3D12_VIEWPORT FRenderTarget::GetViewportMipLevel(int i) const
{
	D3D12_VIEWPORT Viewport = GetViewport();
	Viewport.Width = static_cast<float>(max(1.0f, Viewport.Width / pow(2, i)));
	Viewport.Height = static_cast<float>(max(1.0f, Viewport.Height / pow(2, i)));
	return Viewport;
}

D3D12_RECT FRenderTarget::GetScissorRectMipLevel(int i) const
{
	D3D12_RECT ScissorRect = GetScissorRect();
	ScissorRect.right = static_cast<int>(max(1, ScissorRect.right / pow(2, i)));
	ScissorRect.bottom = static_cast<int>(max(1, ScissorRect.bottom / pow(2, i)));
	return ScissorRect;
}

void FRenderTarget::Initialize(std::vector<std::string> Names)
{
	BuildResource(Names);
	BuildRTHeapAndDSVHeap();
	BuildDescriptors();
}

void FRenderTarget::BuildResource(std::vector<std::string> Names)
{
	// Note, compressed formats cannot be used for UAV.  We get error like:
	// ERROR: ID3D11Device::CreateTexture2D: The format (0x4d, BC3_UNORM) 
	// cannot be bound as an UnorderedAccessView, or cast to a format that
	// could be bound as an UnorderedAccessView.  Therefore this format 
	// does not support D3D11_BIND_UNORDERED_ACCESS.

	D3D12_RESOURCE_DESC TextureDesc;
	ZeroMemory(&TextureDesc, sizeof(D3D12_RESOURCE_DESC));
	TextureDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	TextureDesc.Alignment = 0;
	TextureDesc.Width = mWidth;
	TextureDesc.Height = mHeight;
	TextureDesc.DepthOrArraySize = 1;
	TextureDesc.MipLevels = mMipLevels;
	TextureDesc.Format = mFormat;
	TextureDesc.SampleDesc.Count = 1;
	TextureDesc.SampleDesc.Quality = 0;
	TextureDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	TextureDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;

	FDXResourceManager* DXManager = GetDXResourceManagerPtr();
	ID3D12Device* Device = DXManager->GetDevicePtr();
	D3D12_HEAP_PROPERTIES DefaultHeapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);

	D3D12_CLEAR_VALUE OptClear;
	OptClear.Format = mFormat;
	OptClear.Color[0] = sClearColor[0];
	OptClear.Color[1] = sClearColor[1];
	OptClear.Color[2] = sClearColor[2];
	OptClear.Color[3] = sClearColor[3];

	FTextureManager* TexManager = GetTextureManager();
	for (int i = 0; i < Names.size(); ++i)
	{
		std::unique_ptr<FTexture> Texture = std::make_unique<FTexture>();
		THROW_IF_FAILED(
			Device->CreateCommittedResource(
				&DefaultHeapProperties,
				D3D12_HEAP_FLAG_NONE,
				&TextureDesc,
				D3D12_RESOURCE_STATE_COMMON,
				&OptClear,
				IID_PPV_ARGS(Texture->Resource.GetAddressOf())
			)
		);
		Texture->Name = Names[i];
		mResourceMap[Names[i]].Texture = Texture.get();
		mResourceMap[Names[i]].Index = i;
		mResourceMap[Names[i]].ResourceState = D3D12_RESOURCE_STATE_COMMON;
		TexManager->RegisterTexture2D(std::move(Texture));
	}
	

	// Build Depth Stencil Buffer
	D3D12_RESOURCE_DESC DepthStencilDesc;
	ZeroMemory(&DepthStencilDesc, sizeof(D3D12_RESOURCE_DESC));
	DepthStencilDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	DepthStencilDesc.Alignment = 0;
	DepthStencilDesc.Width = mWidth;
	DepthStencilDesc.Height = mHeight;
	DepthStencilDesc.DepthOrArraySize = 1;
	DepthStencilDesc.MipLevels = 1;
	DepthStencilDesc.Format = mDepthStencilFormat;
	DepthStencilDesc.SampleDesc.Count = 1;
	DepthStencilDesc.SampleDesc.Quality = 0;
	DepthStencilDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	DepthStencilDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

	OptClear.Format = mDepthStencilFormat;
	OptClear.DepthStencil.Depth = 1.0f;
	OptClear.DepthStencil.Stencil = 0;
	THROW_IF_FAILED(
		Device->CreateCommittedResource(
			&DefaultHeapProperties,
			D3D12_HEAP_FLAG_NONE,
			&DepthStencilDesc,
			D3D12_RESOURCE_STATE_COMMON,
			&OptClear,
			IID_PPV_ARGS(mDepthStencilResource.GetAddressOf())
		)
	);
	mDepthStencilState = D3D12_RESOURCE_STATE_COMMON;
}

void FRenderTarget::BuildRTHeapAndDSVHeap()
{
	FDXResourceManager* DXManager = GetDXResourceManagerPtr();
	ID3D12Device* Device = DXManager->GetDevicePtr();
	// RTVHeap
	D3D12_DESCRIPTOR_HEAP_DESC RTVHeapDesc;
	RTVHeapDesc.NumDescriptors = (UINT)mResourceMap.size() * mMipLevels;
	RTVHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	RTVHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	RTVHeapDesc.NodeMask = 0;
	THROW_IF_FAILED(
		Device->CreateDescriptorHeap(
			&RTVHeapDesc,
			IID_PPV_ARGS(mRTVHeap.GetAddressOf())
		)
	);

	// DSVHeap
	D3D12_DESCRIPTOR_HEAP_DESC DSVHeapDesc;
	DSVHeapDesc.NumDescriptors = 1;
	DSVHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	DSVHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	DSVHeapDesc.NodeMask = 0;
	THROW_IF_FAILED(
		Device->CreateDescriptorHeap(
			&DSVHeapDesc,
			IID_PPV_ARGS(mDSVHeap.GetAddressOf())
		)
	);

}

void FRenderTarget::BuildDescriptors()
{
	FDXResourceManager* DXManager = GetDXResourceManagerPtr();
	ID3D12Device* Device = DXManager->GetDevicePtr();

	D3D12_RENDER_TARGET_VIEW_DESC RTVDesc;
	RTVDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
	RTVDesc.Format = mFormat;
	RTVDesc.Texture2D.PlaneSlice = 0;
	for (auto& MapData : mResourceMap)
	{
		FTexture* Texture = MapData.second.Texture;
		ID3D12Resource* Resource = Texture->Resource.Get();
		for (UINT i = 0; i < mMipLevels; ++i)
		{
			RTVDesc.Texture2D.MipSlice = i;
			Device->CreateRenderTargetView(Resource, &RTVDesc, GetRTV(Texture->Name, i));
		}
	}

	Device->CreateDepthStencilView(
		mDepthStencilResource.Get(),
		nullptr,
		mDSVHeap->GetCPUDescriptorHandleForHeapStart()
	);
}
