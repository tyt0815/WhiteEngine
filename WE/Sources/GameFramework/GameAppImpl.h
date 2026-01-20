#pragma once

#include "GameCore.h"
#include "Physics/PhysicsCore.h"
#include "Asset/AssetManager.h"
#include "Render/MeshGeometry.h"

class FGameAppImpl : public GameCore::IGameApp
{
public:
	FGameAppImpl(class WWorld* World);

	virtual ~FGameAppImpl() override;

	virtual void Startup() override;

	virtual void Cleanup() override;

	virtual void Update(float DeltaTime) override;

private:
	class WWorld* mWorld;
	class FDeferredShadingSceneRenderer* mRenderer;
};

#define CREATE_APPLICATION_BY_WORLD(WorldClass)\
	class WorldClass;\
	int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE prevInst, PSTR CmdLine, int nCmdShow)\
	{\
		Physics::Startup();\
		GetMeshGeometryManager();\
		FAssetManager::GetInstance()->LoadAssets();\
		FGameAppImpl App(new WorldClass());\
		return GameCore::RunApplication(App, L#WorldClass, hInstance, nCmdShow);\
	}