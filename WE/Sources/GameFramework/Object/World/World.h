#pragma once

#include "DirectX/DXUtility.h"
#include "Render/MeshGeometry.h"
#include "Render/Material.h"
#include "GameFramework/Object/Pawn/Pawn.h"

class WWorld : public WObject
{
public:
	WWorld();
	virtual ~WWorld() override;
	virtual void Tick(float Delta);
	void SetPlayer(APawn* Player);

protected:
	template<typename T>
	T* SpawnActor();

private:
	TPool<AActor*> mAllActors;
	APawn* mPlayer = nullptr;

public:
	inline const TPool<AActor*>& GetAllActorsRef()
	{
		return mAllActors;
	}
	inline APawn* GetPlayer() const
	{
		return mPlayer;
	}
	inline WCameraComponent* GetPlayerCamera() const
	{
		return mPlayer->GetCameraComponent();
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
