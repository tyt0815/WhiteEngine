#include "DirectX/DXWindow.h"
#include "Render/ForwardRenderer.h"
#include "Render/DeferredRenderer.h"
#include "Render/MeshGeometry.h"

int WINAPI WinMain(
    HINSTANCE hInstance,
    HINSTANCE prevInstance,
    PSTR cmdLine,
    int showCmd
)
{
    // Enable run-time memory check for debug builds.
#if defined(DEBUG) | defined(_DEBUG)
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif
    try
    {
        if (!FDXWindow::GetInstance()->Initialize())
        {
            return 0;
        }
        FMeshGeometry::BuildMeshGeometries();
        FMaterial::BuildMaterial();
        return FDXWindow::GetInstance()->Run();
    }
    catch (UDXException& e)
    {
        MessageBox(nullptr, e.ToString().c_str(), L"HR Failed", MB_OK);
        return 0;
    }
    return 0;
}