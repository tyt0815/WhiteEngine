#include "Main.h"

#include "Render/Renderer.h"

int FMain::Main()
{
	FRenderer Direct3D12App;
	Direct3D12App.Initialize();
	return Direct3D12App.Execute();
}
