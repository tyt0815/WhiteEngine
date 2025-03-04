#pragma once

#include "DirectX/DXUtility.h"
#include "Runtime/Object/Actor.h"
#include "Render/MeshGeometry.h"
#include "Render/Material.h"
#include "Runtime/Object/ViewCamera.h"

class WWorld
{
public:
	WWorld();
	~WWorld();
	virtual bool Initialize();
	virtual void Tick(float Delta);
	inline const std::vector<std::unique_ptr<AActor>>& GetAllActorsRef() { return AllActors; }
	inline const std::vector<std::vector<AActor*>>& GetActorsRef() { return Actors; }
	inline WViewCamera* GetCamera() { return &Camera; }

protected:
	virtual void BuildWorldActors() = 0;
	template<typename T>
	void SpawnActor(EActorType ActorType, FTransform Transform = FTransform::Default);

	std::vector<std::unique_ptr<AActor>> AllActors;
	std::vector<std::vector<AActor*>> Actors;

private:
	WViewCamera Camera;
};

template<typename T>
inline void WWorld::SpawnActor(EActorType ActorType, FTransform Transform)
{
	std::unique_ptr<AActor> Actor = std::make_unique<T>();
	Actor->SetTransform(Transform);
	Actor->ObjectConstantBufferIndex = (UINT)AllActors.size();
	Actors[(int)ActorType].push_back(Actor.get());
	AllActors.push_back(move(Actor));
}