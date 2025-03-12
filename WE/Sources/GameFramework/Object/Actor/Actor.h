#pragma once

#include "GameFramework/Object/Object.h"
#include <d3d12.h>
#include "GameFramework/Object/Component/SceneComponent.h"

extern const int FrameResourcesNum;

class FMeshGeometry;
class FMaterial;

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
	void SetupSceneComponent(WSceneComponent* Component);
	WSceneComponent* mRootComponent = nullptr;
	std::uint64_t mRootComponentPoolId = -1;

public:
	inline void SetTransform(FTransform Transform)
	{
		mRootComponent->SetTransform(Transform);
	}
};

template<typename T>
inline T* AActor::CreateSceneComponent()
{
	T* SceneComponent = GetWObjectManager()->CreateWObject<T>();
	SetupSceneComponent(SceneComponent);
	return SceneComponent;
}

template<typename T>
inline T* AActor::CreateNoneSceneComponent()
{
	T* NoneSceneComponent = GetWObjectManager()->CreateWObject<T>();
	return NoneSceneComponent;
}
