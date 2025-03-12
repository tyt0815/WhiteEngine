#pragma once

#include "DirectX/DXUtility.h"
#include "Render/MeshGeometry.h"
#include "Render/Material.h"
#include "GameFramework/Object/Object.h"

class WWorld : public WObject
{
public:
	WWorld();
	virtual ~WWorld() override;
	virtual bool Initialize();
	virtual void Tick(float Delta);

protected:
	virtual void BuildWorldActors() = 0;
	template<typename T>
	T* SpawnActor();

private:
	TPool<AActor*> mAllActors;
public:
	inline const TPool<AActor*>& GetAllActorsRef()
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
	mAllActors.Register(Actor);
	return Actor;
}
