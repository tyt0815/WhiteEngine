#include "Render/ForwardRenderer.h"
#include "Render/DeferredRenderer.h"
#include "Render/MeshGeometry.h"
#include "Application/TestApplication.h"

#pragma comment(lib,"d3dcompiler.lib")
#pragma comment(lib, "D3D12.lib")
#pragma comment(lib, "dxgi.lib")

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

    //FForwardRenderer Renderer;
    //try
    //{
    //    if (!Renderer.Initialize())
    //    {
    //        return 0;
    //    }
    //    return Renderer.Run();
    //}
    //catch (FDXException& e)
    //{
    //    MessageBox(nullptr, e.ToString().c_str(), L"HR Failed", MB_OK);
    //    return 0;
    //}
    //return 0;
}