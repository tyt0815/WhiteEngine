#include "Main.h"

#include "DirectX12/Direct3D12.h"
#include "Common/Timer.h"

int FMain::Main()
{
	FWindow MainWindow(L"WEngine");
	FDirect3D12 Direct3D12App;
	Direct3D12App.Initialize(&MainWindow);
	return Direct3D12App.Run();
}
