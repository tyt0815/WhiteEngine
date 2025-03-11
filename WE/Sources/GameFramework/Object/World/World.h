#pragma once

#include "DirectX/DXUtility.h"
#include "GameFramework/Object/Actor/ViewCamera.h"
#include "Render/MeshGeometry.h"
#include "Render/Material.h"
#include "GameFramework/Object/Object.h"

class WWorld
{
public:
	WWorld();
	~WWorld();
	virtual bool Initialize();
	virtual void Tick(float Delta);
	inline WViewCamera* GetCamera() { return &Camera; }

protected:
	virtual void BuildWorldActors() = 0;
	template<typename T>
	T* SpawnActor();

private:
	std::vector<AActor*> mAllActors;
	WViewCamera Camera;
public:
	inline const std::vector<AActor*>& GetAllActorsRef()
	{
		return mAllActors;
	}
};

extern WWorld* gWorld;

inline WWorld* GetWorld()
{
	return gWorld;
}

template<typename T>
inline T* WWorld::SpawnActor()
{
	T* Actor = GetWObjectManager()->CreateWObject<T>();
	static_cast<AActor*>(Actor)->ObjectConstantBufferIndex = (UINT)mAllActors.size();
	mAllActors.push_back(Actor);
	return Actor;
}
