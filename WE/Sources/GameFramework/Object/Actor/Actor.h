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
	inline FTransform GetTransform() const
	{
		return mRootComponent->GetTransform();
	}
	inline void SetTransform(FTransform Transform)
	{
		mRootComponent->SetTransform(Transform);
	}
	inline XMFLOAT3 GetLocation() const
	{
		return mRootComponent->GetLocation();
	}
	inline void SetLocation(XMFLOAT3 Location)
	{
		mRootComponent->SetLocation(Location);
	}
	inline XMFLOAT3 GetRotation() const
	{
		return mRootComponent->GetRotation();
	}
	inline void SetRotation(XMFLOAT3 Rotation)
	{
		mRootComponent->SetRotation(Rotation);
	}
	inline XMFLOAT3 GetScale() const
	{
		return mRootComponent->GetScale();
	}
	inline void SetScale(XMFLOAT3 Scale)
	{
		mRootComponent->SetScale(Scale);
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
