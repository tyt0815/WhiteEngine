#include "World.h"
#include "Render/GeometryGenerator.h"
#include "GameFramework/Object/Pawn/Pawn.h"

WWorld* gWorld = nullptr;

WWorld::WWorld()
{
	if (gWorld)
	{
		throw L"이미 월드가 하나 생성되었습니다.";
	}
	gWorld = this;
}

WWorld::~WWorld()
{
	gWorld = nullptr;
}

void WWorld::Tick(float Delta)
{

}

void WWorld::SetPlayer(APawn* Player)
{
	mPlayer = Player;
	mPlayer->SetupPlayerInput();
}
