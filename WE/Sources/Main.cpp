#include "Application/TestApplication.h"
#include "DirectX/DXException.h"
#include "Render/MeshGeometry.h"
#include "Render/Shader.h"

#pragma comment(lib,"d3dcompiler.lib")
#pragma comment(lib, "D3D12.lib")
#pragma comment(lib, "dxgi.lib")

void InitializeSingleton();

HINSTANCE AppInstance;

int WINAPI WinMain(
    HINSTANCE hInstance,
    HINSTANCE prevInstance,
    PSTR cmdLine,
    int showCmd
)
{
    AppInstance = hInstance;
    // Enable run-time memory check for debug builds.
#if defined(DEBUG) | defined(_DEBUG)
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif
    InitializeSingleton();
    FTestApplication* App = FTestApplication::GetInstance();
    try
    {
        if (!App->Initialize())
        {
            return 0;
        }
        return App->Run();
    }
    catch (FDXException& e)
    {
        MessageBox(nullptr, e.ToString().c_str(), L"HR Failed", MB_OK);
        return 0;
    }
}

void InitializeSingleton()
{
    GetMainWindowPtr();
    GetDXResourceManagerPtr();
    GetTextureManager();
    GetMaterialManager();
    GetMeshGeometryManager();
    GetFrameResourceManager();
    GetShaderManager();
}
