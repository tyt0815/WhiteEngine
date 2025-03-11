#include "FrameResource.h"
#include <DirectXColors.h>
#include "Texture.h"
#include "DirectX/DXResourceManager.h"
#include "DirectX/DXException.h"
#include "GameFramework/Object/World/World.h"
#include "Utility/Timer.h"

const int FrameResourcesNum = FRAME_RESOURCES_NUM;

FFrameResource::FFrameResource()
{
	ID3D12Device* Device = GetDXResourceManagerPtr()->GetDevicePtr();
	THROW_IF_FAILED(
		Device->CreateCommandAllocator(
			D3D12_COMMAND_LIST_TYPE_DIRECT,
			IID_PPV_ARGS(CommandAllocator.GetAddressOf())
		)
	);
	PassConstantBuffer = std::make_unique<TUploadBuffer<FPassConstants>>(Device, (UINT)PASS_COUNT, true);
	ObjectConstantBuffer = std::make_unique<TUploadBuffer<FObjectConstants>>(Device, 1, true);
	MaterialConstantBuffer = std::make_unique<TUploadBuffer<FMaterialConstants>>(Device, EMT_None, true);
}

void FFrameResource::Update()
{
	ID3D12Device* Device = GetDXResourceManagerPtr()->GetDevicePtr();
	if (PassConstantBuffer->GetElementCount() < PASS_COUNT)
	{
		PassConstantBuffer = std::make_unique<TUploadBuffer<FPassConstants>>(Device, 1, true);
	}
	if (ObjectConstantBuffer->GetElementCount() < GetWorld()->GetAllActorsRef().size())
	{
		ObjectConstantBuffer = std::make_unique<TUploadBuffer<FObjectConstants>>(Device, (UINT)GetWorld()->GetAllActorsRef().size(), true);
	}
	if (MaterialConstantBuffer->GetElementCount() < EMT_None)
	{
		MaterialConstantBuffer = std::make_unique<TUploadBuffer<FMaterialConstants>>(Device, EMT_None, true);
	}
}

FFrameResourceManager::FFrameResourceManager()
{
	for (int i = 0; i < FrameResourcesNum; ++i)
	{
		mFrameResources.push_back(std::make_unique<FFrameResource>());
	}
	mTargetFrameResource = mFrameResources[mTargetFrameResourceIndex].get();
	BuildRootSignature();
}

FFrameResourceManager::~FFrameResourceManager()
{

}

void FFrameResourceManager::Tick()
{
    SetTargetFrameResource();
	mTargetFrameResource->Update();
	UpdatePassCB();
	UpdateObjectCB();
	UpdateMaterialCB();
}

void FFrameResourceManager::SetTargetFrameResource()
{
	FDXResourceManager* DXResourceManager = GetDXResourceManagerPtr();
	DXResourceManager->SignalFence();
	mTargetFrameResource->Fence = DXResourceManager->GetCurrentFence();
	// Cycle through the circular frame resource array.
	mTargetFrameResourceIndex = (mTargetFrameResourceIndex + 1) % FrameResourcesNum;
	mTargetFrameResource = mFrameResources[mTargetFrameResourceIndex].get();

	// Has the GPU finished processing the commands of the current frame resource?
	// If not, wait until the GPU has completed commands up to this fence point.
	ID3D12Fence* Fence = DXResourceManager->GetFencePtr();
	if (mTargetFrameResource->Fence != 0 && Fence->GetCompletedValue() < mTargetFrameResource->Fence)
	{
		HANDLE eventHandle = CreateEventEx(nullptr, nullptr, false, EVENT_ALL_ACCESS);
		THROW_IF_FAILED(Fence->SetEventOnCompletion(mTargetFrameResource->Fence, eventHandle));
		WaitForSingleObject(eventHandle, INFINITE);
		CloseHandle(eventHandle);
	}
}

void FFrameResourceManager::BuildRootSignature()
{
	CD3DX12_DESCRIPTOR_RANGE TextureTable;
	TextureTable.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);

	CD3DX12_ROOT_PARAMETER RootParameter[4];
	RootParameter[0].InitAsConstantBufferView(0);
	RootParameter[1].InitAsConstantBufferView(1);
	RootParameter[2].InitAsConstantBufferView(2);
	RootParameter[3].InitAsDescriptorTable(1, &TextureTable, D3D12_SHADER_VISIBILITY_PIXEL);

	auto StaticSamplers = FTexture::GetStaticSamplers();

	CD3DX12_ROOT_SIGNATURE_DESC RootSignatureDesc(
		4,
		RootParameter,
		(UINT)StaticSamplers.size(),
		StaticSamplers.data(),
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT
	);

	Microsoft::WRL::ComPtr<ID3DBlob> SerializedRootSignature = nullptr;
	Microsoft::WRL::ComPtr<ID3DBlob> ErrorBlob = nullptr;
	HRESULT HResult = D3D12SerializeRootSignature(
		&RootSignatureDesc,
		D3D_ROOT_SIGNATURE_VERSION_1,
		SerializedRootSignature.GetAddressOf(),
		ErrorBlob.GetAddressOf()
	);

	if (ErrorBlob != nullptr)
	{
		::OutputDebugStringA((char*)ErrorBlob->GetBufferPointer());
	}
	THROW_IF_FAILED(HResult);

	ID3D12Device* Device = GetDXResourceManagerPtr()->GetDevicePtr();
	THROW_IF_FAILED(
		Device->CreateRootSignature(
			0,
			SerializedRootSignature->GetBufferPointer(),
			SerializedRootSignature->GetBufferSize(),
			IID_PPV_ARGS(mRootSignature.GetAddressOf())
		)
	)
}

void FFrameResourceManager::UpdatePassCB()
{
	FDXResourceManager* DeviceManager = FDXResourceManager::GetInstance();
	WViewCamera* Camera = GetWorld()->GetCamera();
	UTimer* Timer = GetAppTimer();

	Camera->UpdateViewMatrix();
	// Build the view matrix.
	XMVECTOR target = XMVectorZero();
	XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

	XMMATRIX view = Camera->GetViewMatrix();
	XMMATRIX InvView = FDXMath::GetInverseMatrix(view);
	XMMATRIX proj = Camera->GetProjMatrix();
	XMMATRIX InvProj = FDXMath::GetInverseMatrix(proj);
	XMMATRIX ViewProj = view * proj;
	XMMATRIX InvViewProj = FDXMath::GetInverseMatrix(ViewProj);

	FPassConstants PassConstants;
	XMStoreFloat4x4(&PassConstants.View, XMMatrixTranspose(view));
	XMStoreFloat4x4(&PassConstants.InvView, XMMatrixTranspose(InvView));
	XMStoreFloat4x4(&PassConstants.Proj, XMMatrixTranspose(proj));
	XMStoreFloat4x4(&PassConstants.InvProj, XMMatrixTranspose(InvProj));
	XMStoreFloat4x4(&PassConstants.ViewProj, XMMatrixTranspose(ViewProj));
	XMStoreFloat4x4(&PassConstants.InvViewProj, XMMatrixTranspose(InvViewProj));
	PassConstants.EyePosW = Camera->GetTranslation();
	D3D12_VIEWPORT Viewport = DeviceManager->GetScreenViewport();
	float Width = static_cast<float>(Viewport.Width);
	float Height = static_cast<float>(Viewport.Height);
	PassConstants.RenderTargetSize = XMFLOAT2(Width, Height);
	PassConstants.InvRenderTargetSize = XMFLOAT2(1.0f / Width, 1.0f / Height);
	PassConstants.NearZ = 1.0f;
	PassConstants.FarZ = 1000.0f;
	PassConstants.TotalTime = Timer->GetTotalTime();
	PassConstants.DeltaTime = Timer->GetDeltaTime();

	PassConstants.AmbientLight = { 0.25f, 0.25f, 0.35f, 1.0f };

	PassConstants.FogColor = XMFLOAT4(Colors::LightSkyBlue);
	PassConstants.FogStart = 200.0f;
	PassConstants.FogRange = 100.0f;

	PassConstants.Lights[0].Direction = { 0.57735f, -0.57735f, 0.57735f };
	PassConstants.Lights[0].Strength = { 1.2f, 1.2f, 1.2f };
	PassConstants.Lights[1].Direction = { -0.57735f, -0.57735f, 0.57735f };
	PassConstants.Lights[1].Strength = { 0.3f, 0.3f, 0.3f };
	PassConstants.Lights[2].Direction = { 0.0f, -0.707f, -0.707f };
	PassConstants.Lights[2].Strength = { 0.15f, 0.15f, 0.15f };

	mTargetFrameResource->PassConstantBuffer->CopyData(0, PassConstants);
}

void FFrameResourceManager::UpdateObjectCB()
{
	const auto& Actors = GetWorld()->GetAllActorsRef();
	for (auto& Actor : Actors)
	{
		if (Actor->DirtyFrameCount > 0)
		{
			FObjectConstants Constants;
			XMStoreFloat4x4(&Constants.World, XMMatrixTranspose(Actor->GetWorldMatrix()));
			XMStoreFloat4x4(&Constants.TexTransform, XMMatrixTranspose(Actor->GetTextureTransformMatrix()));
			mTargetFrameResource->ObjectConstantBuffer->CopyData(Actor->ObjectConstantBufferIndex, Constants);
			--Actor->DirtyFrameCount;
		}
	}
}

void FFrameResourceManager::UpdateMaterialCB()
{
	for (std::uint16_t i = 0; i < EMT_None; ++i)
	{
		FMaterial* Material = GetMaterialManager()->GetMaterial(i);
		if (Material->DirtyFrameCount > 0)
		{
			XMMATRIX MaterialTransform = XMLoadFloat4x4(&Material->MatTransform);
			FMaterialConstants MaterialConstants;
			MaterialConstants.DiffuseAlbedo = Material->DiffuseAlbedo;
			MaterialConstants.FresnelR0 = Material->FresnelR0;
			XMStoreFloat4x4(&MaterialConstants.MatTransform, XMMatrixTranspose(MaterialTransform));
			MaterialConstants.Roughness = Material->Roughness;

			mTargetFrameResource->MaterialConstantBuffer->CopyData(Material->MatCBIndex, MaterialConstants);
			--Material->DirtyFrameCount;
		}
	}
}
