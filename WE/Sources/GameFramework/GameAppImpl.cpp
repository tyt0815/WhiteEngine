#include "GameAppImpl.h"
#include "Render/DeferredShadingSceneRenderer.h"
#include "Physics/PhysicsCore.h"
#include "DirectX/DXResourceManager.h"
#include "Object/World/World.h"

FGameAppImpl::FGameAppImpl(WWorld* World):
	mWorld(World)
{
}

FGameAppImpl::~FGameAppImpl()
{
	delete mWorld;
	delete mRenderer;
}

void FGameAppImpl::Startup()
{
	mWorld->BeginPlay();
	mRenderer = new FDeferredShadingSceneRenderer();
	mRenderer->Initialize(GetDXResourceManagerPtr()->GetDevicePtr());
}

void FGameAppImpl::Cleanup()
{
	mRenderer->Destroy();
	Physics::Cleanup();
}

void FGameAppImpl::Update(float DeltaTime)
{
	mWorld->Tick(DeltaTime);

	mRenderer->Tick(mWorld->GetRenderItemProxyPtr());
}