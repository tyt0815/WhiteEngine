#include "Window.h"

#include <windowsx.h>

FWindow gMainWindow = FWindow(L"MainWindow", L"WE", 1920, 1080);

LRESULT CALLBACK WindowProcedure(HWND WindowHandle, UINT Message, WPARAM WParam, LPARAM LParam)
{
	FWindow* Window = (FWindow*)GetWindowLongPtr(WindowHandle, GWLP_USERDATA);
	if (Window) {
		return Window->WindowProcedure(WindowHandle, Message, WParam, LParam);
	}
	return DefWindowProc(WindowHandle, Message, WParam, LParam);
}

LRESULT CALLBACK InitialWindowProcedure(HWND WindowHandle, UINT Message, WPARAM WParam, LPARAM LParam)
{
	if (Message == WM_NCCREATE) {
		CREATESTRUCT* Create = (CREATESTRUCT*)LParam;
		FWindow* Window = (FWindow*)Create->lpCreateParams;
		SetWindowLongPtr(WindowHandle, GWLP_USERDATA, (LONG_PTR)Window);
		SetWindowLongPtr(WindowHandle, GWLP_WNDPROC, (LONG_PTR)WindowProcedure);
		return Window->WindowProcedure(WindowHandle, Message, WParam, LParam);
	}
	return DefWindowProc(WindowHandle, Message, WParam, LParam);
}

FWindow::FWindow(const std::wstring ClassName, const std::wstring WindowName, UINT Width, UINT Height)
{
	mClassName = ClassName;
	mWindowName = WindowName;
	WNDCLASS WndClass;
	WndClass.style = CS_HREDRAW | CS_VREDRAW;
	WndClass.lpfnWndProc = InitialWindowProcedure;
	WndClass.cbClsExtra = 0;
	WndClass.cbWndExtra = 0;
	WndClass.hInstance = AppInstance;
	WndClass.hIcon = LoadIcon(0, IDI_APPLICATION);
	WndClass.hCursor = LoadCursor(0, IDC_ARROW);
	WndClass.hbrBackground = (HBRUSH)GetStockObject(NULL_BRUSH);
	WndClass.lpszMenuName = 0;
	WndClass.lpszClassName = mClassName.c_str();

	if (!RegisterClass(&WndClass))
	{
		throw L"RegisterClass Failed.";
	}

	// Compute window rectangle dimensions based on requested client area dimensions.
	RECT Rect = { 0, 0, static_cast<LONG>(Width), static_cast<LONG>(Height) };
	AdjustWindowRect(&Rect, WS_OVERLAPPEDWINDOW, false);
	mWidth = static_cast<UINT>(Rect.right - Rect.left);
	mHeight = static_cast<UINT>(Rect.bottom - Rect.top);

	mWindowHandle = CreateWindow(
		mClassName.c_str(),
		mWindowName.c_str(),
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT,
		mWidth, mHeight,
		0, 0, AppInstance, this
	);
	if (!mWindowHandle)
	{
		throw "CreateWindow Failed.";
	}

	// 마우스 이동량을 알기 위해 rawinputdevice 등록
	RAWINPUTDEVICE Rid;
	Rid.usUsagePage = 0x01; // Generic Desktop Controls
	Rid.usUsage = 0x02;     // Mouse
	Rid.dwFlags = RIDEV_INPUTSINK; // 백그라운드에서도 입력 받기
	Rid.hwndTarget = mWindowHandle; // 메시지를 받을 윈도우 핸들

	if (!RegisterRawInputDevices(&Rid, 1, sizeof(RAWINPUTDEVICE)))
	{
		MessageBox(0, L"Failed to register raw input", L"Error", MB_OK);
	}

	ShowWindow(mWindowHandle, SW_SHOW);
	UpdateWindow(mWindowHandle);
}

LRESULT FWindow::WindowProcedure(HWND WindowHandle, UINT Message, WPARAM WParam, LPARAM LParam)
{
	// 화면 중앙 좌표
	RECT R;
	::GetClientRect(WindowHandle, &R);
	POINT CenterPos;
	CenterPos.x = (R.left + R.right) / 2;
	CenterPos.y = (R.top + R.bottom) / 2;

	// Input 처리
	if (bCaptured)
	{
		switch (Message)
		{
		case WM_LBUTTONDOWN:
			GetInputSystemManager()->ProcessMouseInput(EMIT_LDown, GET_X_LPARAM(LParam), GET_Y_LPARAM(LParam));
			break;
		case WM_MBUTTONDOWN:
			GetInputSystemManager()->ProcessMouseInput(EMIT_MDown, GET_X_LPARAM(LParam), GET_Y_LPARAM(LParam));
			break;
		case WM_RBUTTONDOWN:
			GetInputSystemManager()->ProcessMouseInput(EMIT_RDown, GET_X_LPARAM(LParam), GET_Y_LPARAM(LParam));
			break;
		case WM_LBUTTONUP:
			GetInputSystemManager()->ProcessMouseInput(EMIT_LUp, GET_X_LPARAM(LParam), GET_Y_LPARAM(LParam));
			break;
		case WM_MBUTTONUP:
			GetInputSystemManager()->ProcessMouseInput(EMIT_MUp, GET_X_LPARAM(LParam), GET_Y_LPARAM(LParam));
			break;
		case WM_RBUTTONUP:
			GetInputSystemManager()->ProcessMouseInput(EMIT_RUp, GET_X_LPARAM(LParam), GET_Y_LPARAM(LParam));
			break;
		case WM_INPUT:
		{
			UINT dwSize = 0;
			GetRawInputData((HRAWINPUT)LParam, RID_INPUT, nullptr, &dwSize, sizeof(RAWINPUTHEADER));

			BYTE* lpb = new BYTE[dwSize];
			if (GetRawInputData((HRAWINPUT)LParam, RID_INPUT, lpb, &dwSize, sizeof(RAWINPUTHEADER)) == dwSize)
			{
				RAWINPUT* raw = (RAWINPUT*)lpb;
				if (raw->header.dwType == RIM_TYPEMOUSE)
				{
					int dX = raw->data.mouse.lLastX;
					int dY = raw->data.mouse.lLastY;

					GetInputSystemManager()->ProcessMouseInput(EMIT_Move, dX, dY);
				}
			}
			delete[] lpb;


			//static int LastX = GET_X_LPARAM(LParam);
			//static int LastY = GET_Y_LPARAM(LParam);
			//int dX = GET_X_LPARAM(LParam) - LastX;
			//int dY = GET_Y_LPARAM(LParam) - LastY;
			//GetInputSystemManager()->ProcessMouseInput(EMIT_Move, dX, dY);
			//LastX = GET_X_LPARAM(LParam);
			//LastY = GET_Y_LPARAM(LParam);
			break;
		}

		case WM_MOUSEWHEEL:
			GetInputSystemManager()->ProcessMouseInput(EMIT_Wheel, GET_X_LPARAM(LParam), GET_Y_LPARAM(LParam));
			break;
		}
	}


	switch (Message)
	{
		// WM_ACTIVATE is sent when the window is activated or deactivated.  
		// We pause the game when the window is deactivated and unpause it 
		// when it becomes active.  
	case WM_ACTIVATE:
		if (LOWORD(WParam) == WA_INACTIVE)
		{
			bPaused = true;
		}
		else
		{
			bPaused = false;
		}
		return 0;

		// WM_SIZE is sent when the user resizes the window.  
	case WM_SIZE:
		UpdateWindowSize();
		// Save the new client area dimensions.
		if (WParam == SIZE_MINIMIZED)
		{
			bPaused = true;
			bMinimized = true;
			bMaximized = false;
		}
		else if (WParam == SIZE_MAXIMIZED)
		{
			bPaused = false;
			bMinimized = false;
			bMaximized = true;
			Resize();
		}
		else if (WParam == SIZE_RESTORED)
		{

			// Restoring from minimized state?
			if (bMinimized)
			{
				bPaused = false;
				bMinimized = false;
				Resize();
			}

			// Restoring from maximized state?
			else if (bMaximized)
			{
				bPaused = false;
				bMaximized = false;
				Resize();
			}
			else if (bResized)
			{
				// If user is dragging the resize bars, we do not resize 
				// the buffers here because as the user continuously 
				// drags the resize bars, a stream of WM_SIZE messages are
				// sent to the window, and it would be pointless (and slow)
				// to resize for each WM_SIZE message received from dragging
				// the resize bars.  So instead, we reset after the user is 
				// done resizing the window and releases the resize bars, which 
				// sends a WM_EXITSIZEMOVE message.
			}
			else // API call such as SetWindowPos or mSwapChain->SetFullscreenState.
			{
				Resize();
			}
		}
		return 0;

		// WM_EXITSIZEMOVE is sent when the user grabs the resize bars.
	case WM_ENTERSIZEMOVE:
		bPaused = true;
		bResized = true;
		return 0;

		// WM_EXITSIZEMOVE is sent when the user releases the resize bars.
		// Here we reset everything based on the new window dimensions.
	case WM_EXITSIZEMOVE:
		bPaused = false;
		bResized = false;
		UpdateWindowSize();
		Resize();
		return 0;

		// WM_DESTROY is sent when the window is being destroyed.
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;

		// The WM_MENUCHAR message is sent when a menu is active and the user presses 
		// a key that does not correspond to any mnemonic or accelerator key. 
	case WM_MENUCHAR:
		// Don't beep when we alt-enter.
		return MAKELRESULT(0, MNC_CLOSE);

		// Catch this message so to prevent the window from becoming too small.
	case WM_GETMINMAXINFO:
		((MINMAXINFO*)LParam)->ptMinTrackSize.x = 200;
		((MINMAXINFO*)LParam)->ptMinTrackSize.y = 200;
		return 0;

	case WM_LBUTTONDOWN:
	case WM_MBUTTONDOWN:
	case WM_RBUTTONDOWN:
		OnMouseDown(WParam, GET_X_LPARAM(LParam), GET_Y_LPARAM(LParam));
		return 0;
	case WM_LBUTTONUP:
	case WM_MBUTTONUP:
	case WM_RBUTTONUP:
		return 0;
	case WM_MOUSEMOVE:
		return 0;
	case WM_KEYDOWN:
		OnKeyDown(WParam);
		return 0;
	case WM_KEYUP:
		return 0;
	case WM_MOUSEWHEEL:
		return 0;
	}	

	// 마우스를 화면 중앙에 고정
	if (bCaptured)
	{
		::ClientToScreen(WindowHandle, &CenterPos);
		::SetCursorPos(CenterPos.x, CenterPos.y);
	}

	return DefWindowProc(WindowHandle, Message, WParam, LParam);
}

void FWindow::UpdateWindowSize()
{
	RECT R;
	::GetClientRect(mWindowHandle, &R);
	mWidth = R.right - R.left;
	mHeight = R.bottom - R.top;
}

void FWindow::Resize()
{
	for (std::function<void()>& Function : mResizeCallbackFunctions)
	{
		Function();
	}
}

void FWindow::OnMouseDown(WPARAM WParam, int X, int Y)
{
	if (!bCaptured)
	{
		bCaptured = true;
		SetCapture(mWindowHandle);
		ShowCursor(false);
	}
}

void FWindow::OnKeyDown(WPARAM WParam)
{
	if (WParam == VK_ESCAPE && bCaptured)
	{
		bCaptured = false;
		ReleaseCapture();
		ShowCursor(true);
	}
}