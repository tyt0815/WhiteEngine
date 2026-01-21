#include "GameAppImpl.h"
#include "Render/DeferredShadingSceneRenderer.h"
#include "Physics/PhysicsCore.h"
#include "DirectX/DXResourceManager.h"
#include "Object/World/World.h"

#include "GUI/GUICore.h"
#include "Utility/Timer.h"

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
	UTimer Timer;
	mWorld->Tick(DeltaTime);
	Timer.Tick();
	
	mRenderer->Tick(mWorld->GetRenderItemProxyPtr());
	Timer.Tick();
	double RenderDelta = Timer.GetDeltaTime();
	// 프로파일링용 GUI
	GUI::FDrawCommand Command;
	Command.LifeSpan = 0;
	Command.DrawLambda = [=]()
	{
		ImGui::TextColored(ImVec4(1, 1, 0, 1), "Rendering"); // 노란색 제목
		ImGui::Separator();
		ImGui::Text("Rendering: %.8f", RenderDelta);
	};
	GUI::AddProfilingCommand(Command);
}