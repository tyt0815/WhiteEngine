#pragma once

#include "GameFramework/Object/Object.h"
#include <d3d12.h>
#include "GameFramework/Object/Component/SceneComponent.h"

extern const int FrameResourcesNum;

class FMeshGeometry;
class FMaterial;
class WCameraComponent;

class AActor : public WObject
{
public:
	virtual void Tick(float Delta) override;
	template<typename T>
	T* CreateSceneComponent();
	template<typename T>
	T* CreateNoneSceneComponent();
	void SetRootComponent(WSceneComponent* Component);

protected:

private:
	void SetupComponent(WActorComponent* Component);
	void SetupSceneComponent(WSceneComponent* Component);
	WSceneComponent* mRootComponent = nullptr;
	std::uint64_t mRootComponentPoolId = -1;
	WCameraComponent* mCameraComponent = nullptr;

public:
	inline WSceneComponent* GetRootComponent() const
	{
		return mRootComponent;
	}
	inline FTransform GetTransform() const
	{
		return mRootComponent->GetTransform();
	}
	inline void SetTransform(FTransform Transform)
	{
		mRootComponent->SetTransform(Transform);
	}
	inline WCameraComponent* GetCameraComponent() const
	{
		return mCameraComponent;
	}
	inline void SetCameraComponent(WCameraComponent* CameraComponent)
	{
		mCameraComponent = CameraComponent;
	}
};

template<typename T>
inline T* AActor::CreateSceneComponent()
{
	T* SceneComponent = GetWObjectManager()->CreateWObject<T>();
	SetupComponent(SceneComponent);
	SetupSceneComponent(SceneComponent);
	return SceneComponent;
}

template<typename T>
inline T* AActor::CreateNoneSceneComponent()
{
	T* NoneSceneComponent = GetWObjectManager()->CreateWObject<T>();
	SetupComponent(NoneSceneComponent);
	return NoneSceneComponent;
}
