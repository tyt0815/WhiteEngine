#pragma once

#include "GameFramework/Object/Object.h"
#include <d3d12.h>
#include "GameFramework/Object/Component/SceneComponent.h"

extern const int gFrameResourcesNum;

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
	XMFLOAT3 GetFowardVector() const;
	XMFLOAT3 GetRightVector() const;
	XMFLOAT3 GetUpVector() const;

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
	inline FTransform GetActorTransform() const
	{
		return mRootComponent->GetLocalTransform();
	}
	inline void SetActorTransform(FTransform Transform)
	{
		mRootComponent->SetLocalTransform(Transform);
	}
	inline XMFLOAT3 GetActorLocation() const
	{
		return mRootComponent->GetLocalLocation();
	}
	inline void SetActorLocation(XMFLOAT3 Location)
	{
		mRootComponent->SetLocalLocation(Location);
	}
	inline XMFLOAT3 GetActorRotation() const
	{
		return mRootComponent->GetLocalRotation();
	}
	inline void SetActorRotation(XMFLOAT3 Rotation)
	{
		mRootComponent->SetLocalRotation(Rotation);
	}
	inline XMFLOAT3 GetActorScale() const
	{
		return mRootComponent->GetLocalScale();
	}
	inline void SetActorScale(XMFLOAT3 Scale)
	{
		mRootComponent->SetLocalScale(Scale);
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
