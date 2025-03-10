#include "FrameResource.h"
#include <DirectXColors.h>
#include "Material.h"
#include "DirectX/DXDeviceManager.h"
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
	PassConstantBuffer = std::make_unique<TUploadBuffer<FPassConstants>>(Device, 1, true);
	ObjectConstantBuffer = std::make_unique<TUploadBuffer<FObjectConstants>>(Device, (UINT)GetWorld()->GetAllActorsRef().size(), true);
	MaterialConstantBuffer = std::make_unique<TUploadBuffer<FMaterialConstants>>(Device, (UINT)FMaterial::Materials.size(), true);
}

void FFrameResource::Update()
{
	ID3D12Device* Device = GetDXResourceManagerPtr()->GetDevicePtr();
	if (PassConstantBuffer->GetElementCount() < 1)
	{
		PassConstantBuffer = std::make_unique<TUploadBuffer<FPassConstants>>(Device, 1, true);
	}
	if (ObjectConstantBuffer->GetElementCount() < GetWorld()->GetAllActorsRef().size())
	{
		ObjectConstantBuffer = std::make_unique<TUploadBuffer<FObjectConstants>>(Device, (UINT)GetWorld()->GetAllActorsRef().size(), true);
	}
	if (MaterialConstantBuffer->GetElementCount() < (UINT)FMaterial::Materials.size())
	{
		MaterialConstantBuffer = std::make_unique<TUploadBuffer<FMaterialConstants>>(Device, (UINT)FMaterial::Materials.size(), true);
	}
}

FFrameResourceManager::FFrameResourceManager()
{
	for (int i = 0; i < FrameResourcesNum; ++i)
	{
		mFrameResources.push_back(std::make_unique<FFrameResource>());
	}
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
	// Cycle through the circular frame resource array.
	mTargetFrameResourceIndex = (mTargetFrameResourceIndex + 1) % FrameResourcesNum;
	mTargetFrameResource = mFrameResources[mTargetFrameResourceIndex].get();

	// Has the GPU finished processing the commands of the current frame resource?
	// If not, wait until the GPU has completed commands up to this fence point.
	ID3D12Fence* Fence = GetDXResourceManagerPtr()->GetFencePtr();
	if (mTargetFrameResource->Fence != 0 && Fence->GetCompletedValue() < mTargetFrameResource->Fence)
	{
		HANDLE eventHandle = CreateEventEx(nullptr, nullptr, false, EVENT_ALL_ACCESS);
		THROW_IF_FAILED(Fence->SetEventOnCompletion(mTargetFrameResource->Fence, eventHandle));
		WaitForSingleObject(eventHandle, INFINITE);
		CloseHandle(eventHandle);
	}
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
		if (Actor->NumFramesDirty > 0)
		{
			FObjectConstants Constants;
			XMStoreFloat4x4(&Constants.World, XMMatrixTranspose(Actor->GetWorldMatrix()));
			XMStoreFloat4x4(&Constants.TexTransform, XMMatrixTranspose(Actor->GetTextureTransformMatrix()));
			mTargetFrameResource->ObjectConstantBuffer->CopyData(Actor->ObjectConstantBufferIndex, Constants);
			--Actor->NumFramesDirty;
		}
	}
}

void FFrameResourceManager::UpdateMaterialCB()
{
	for (auto& Item : FMaterial::Materials)
	{
		FMaterial* Material = Item.second.get();
		if (Material->NumFramesDirty > 0)
		{
			XMMATRIX MaterialTransform = XMLoadFloat4x4(&Material->MatTransform);
			FMaterialConstants MaterialConstants;
			MaterialConstants.DiffuseAlbedo = Material->DiffuseAlbedo;
			MaterialConstants.FresnelR0 = Material->FresnelR0;
			XMStoreFloat4x4(&MaterialConstants.MatTransform, XMMatrixTranspose(MaterialTransform));
			MaterialConstants.Roughness = Material->Roughness;

			mTargetFrameResource->MaterialConstantBuffer->CopyData(Material->MatCBIndex, MaterialConstants);
			--Material->NumFramesDirty;
		}
	}
}
