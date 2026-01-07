#include "GameFramework/GameCore.h"
#include "World/TestWorld.h"
#include "Render/DeferredShadingSceneRenderer.h"
#include "DirectX/DXResourceManager.h"


#pragma comment(lib,"d3dcompiler.lib")
#pragma comment(lib, "D3D12.lib")
#pragma comment(lib, "dxgi.lib")

class FTestApp : public GameCore::IGameApp
{
public:
	virtual void Startup() override;

    virtual void Cleanup() override;

	virtual void Update(float DeltaTime) override;


private:
    WTestWorld mWorld;
    FDeferredShadingSceneRenderer mRenderer;
};

CREATE_APPLICATION(FTestApp)

void FTestApp::Startup()
{
    mRenderer.Initialize(GetDXResourceManagerPtr()->GetDevicePtr());
}

void FTestApp::Cleanup()
{
    mRenderer.Destroy();

}

void FTestApp::Update(float DeltaTime)
{
    mWorld.Tick(DeltaTime);

    mRenderer.Tick();
}