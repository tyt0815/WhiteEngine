#include "RendererBase.h"
#include "Runtime/World/World.h"

FRendererBase::FRendererBase(WWorld* InWorld):
    World(InWorld),
    DXWindow(FDXWindow::GetInstance())
{
}

bool FRendererBase::Initialize()
{
    BuildFrameResources();
    BuildDescriptorHeaps();
    BuildConstantBuffers();
	BuildRootSignature();
    BuildShaderAndInputLayout();
    BuildPipelineStateObject();
    return true;
}

void FRendererBase::Render()
{
    SetTargetFrameResource();
    UpdateCamera();
}

void FRendererBase::BuildFrameResources()
{
    ID3D12Device* Device = DXWindow->GetDevice();
	for (int i = 0; i < NumFrameResources; ++i)
	{
		FrameResources.push_back(
			make_unique<FFrameResource>(
				Device,
				1,
				(UINT)World->GetWorldActorsRef().size(),
				(UINT)FMaterial::Materials.size()
			)
		);
	}
}

void FRendererBase::SetTargetFrameResource()
{
    ID3D12Fence* Fence = DXWindow->GetFence();
    // Cycle through the circular frame resource array.
    TargetFrameResourceIndex = (TargetFrameResourceIndex + 1) % NumFrameResources;
    TargetFrameResource = FrameResources[TargetFrameResourceIndex].get();

    // Has the GPU finished processing the commands of the current frame resource?
    // If not, wait until the GPU has completed commands up to this fence point.
    if (TargetFrameResource->Fence != 0 && Fence->GetCompletedValue() < TargetFrameResource->Fence)
    {
        HANDLE eventHandle = CreateEventEx(nullptr, nullptr, false, EVENT_ALL_ACCESS);
        ThrowIfFailed(Fence->SetEventOnCompletion(TargetFrameResource->Fence, eventHandle));
        WaitForSingleObject(eventHandle, INFINITE);
        CloseHandle(eventHandle);
    }
}

void FRendererBase::UpdateCamera()
{
    // Convert Spherical to Cartesian coordinates.
    FDXWindow::FCamera Camera = DXWindow->Camera;
    Camera.EyePos.x = Camera.Radius * sinf(Camera.Phi) * cosf(Camera.Theta);
    Camera.EyePos.z = Camera.Radius * sinf(Camera.Phi) * sinf(Camera.Theta);
    Camera.EyePos.y = Camera.Radius * cosf(Camera.Phi);

    // Build the view matrix.
    XMVECTOR pos = XMVectorSet(Camera.EyePos.x, Camera.EyePos.y, Camera.EyePos.z, 1.0f);
    XMVECTOR target = XMVectorZero();
    XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

    XMMATRIX view = XMMatrixLookAtLH(pos, target, up);
    XMStoreFloat4x4(&Camera.View, view);

    XMMATRIX proj = XMLoadFloat4x4(&Camera.Project);

    FPassConstants PassConstants;
    XMStoreFloat4x4(&PassConstants.View, XMMatrixTranspose(view));
    XMStoreFloat4x4(&PassConstants.Proj, XMMatrixTranspose(proj));
    TargetFrameResource->PassConstantBuffer->CopyData(0, PassConstants);
}
