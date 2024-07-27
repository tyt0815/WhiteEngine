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
	bool Initialize();
	void Tick();
	inline const vector<unique_ptr<AActor>>& GetWorldActorsRef() { return WorldActors; }
	inline const vector<AActor*>& GetOpaqueActorsRef() { return OpaqueActors; }

protected:
	virtual void BuildWorldActors() = 0;

	vector<unique_ptr<AActor>> WorldActors;
	vector<AActor*> OpaqueActors;

private:
	
};