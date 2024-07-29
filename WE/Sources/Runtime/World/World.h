#pragma once

#include "DirectX/DXUtility.h"
#include "Runtime/Object/Actor.h"
#include "Render/MeshGeometry.h"
#include "Render/Material.h"

class WWorld
{
public:
	WWorld();
	~WWorld();
	virtual bool Initialize();
	virtual void Tick();
	inline const vector<unique_ptr<AActor>>& GetWorldActorsRef() { return WorldActors; }

protected:
	virtual void BuildWorldActors() = 0;
	template<typename T>
	void SpawnActor(FTransform Transform = FTransform::Default);

	vector<unique_ptr<AActor>> WorldActors;

private:
	
};

template<typename T>
inline void WWorld::SpawnActor(FTransform Transform)
{
	unique_ptr<AActor> Actor = make_unique<T>();
	Actor->SetTransform(Transform);
	Actor->ObjectConstantBufferIndex = (UINT)WorldActors.size();
	WorldActors.push_back(move(Actor));
}