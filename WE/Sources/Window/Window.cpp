#include "Window.h"

#include <windowsx.h>

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

bool FWindow::Initialize(const std::wstring ClassName, const std::wstring WindowName, UINT Width, UINT Height)
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
		MessageBox(0, L"RegisterClass Failed.", 0, 0);
		return false;
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
		MessageBox(0, L"CreateWindow Failed.", 0, 0);
		return false;
	}

	ShowWindow(mWindowHandle, SW_SHOW);
	UpdateWindow(mWindowHandle);

	mInputActionFunctions.resize((size_t)EInputType::EIT_None);

	return true;
}

LRESULT FWindow::WindowProcedure(HWND WindowHandle, UINT Message, WPARAM WParam, LPARAM LParam)
{
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
		OnMouseUp(WParam, GET_X_LPARAM(LParam), GET_Y_LPARAM(LParam));
		return 0;
	case WM_MOUSEMOVE:
		OnMouseMove(WParam, GET_X_LPARAM(LParam), GET_Y_LPARAM(LParam));
		return 0;
	case WM_KEYDOWN:
		OnKeyDown(WParam);
		return 0;
	case WM_KEYUP:
		OnKeyUp(WParam);
		return 0;
	case WM_MOUSEWHEEL:
		OnMouseWheel(WParam);
		return 0;
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
	mLastX = X;
	mLastY = Y;
	for (std::function<void(WPARAM, FMouseInputParameter&)>& Function : mInputActionFunctions[(int)EInputType::EIT_MouseDown])
	{
		mMouseInputParameter.SetParameters(X, Y, mLastX, mLastY);
		Function(WParam, mMouseInputParameter);
	}


	/*LastMousePos.x = X;
	LastMousePos.y = Y;*/
	if (WParam == MK_RBUTTON)
	{
		SetCapture(mWindowHandle);
		ShowCursor(false);
	}
}

void FWindow::OnMouseUp(WPARAM WParam, int X, int Y)
{
	for (std::function<void(WPARAM, FMouseInputParameter&)>& Function : mInputActionFunctions[(int)EInputType::EIT_MouseUp])
	{
		mMouseInputParameter.SetParameters(X, Y, mLastX, mLastY);
		Function(WParam, mMouseInputParameter);
	}

	//WParam
	ReleaseCapture();
	if (WParam == MK_RBUTTON)
	{
		ReleaseCapture();
		ShowCursor(true);
	}
}

void FWindow::OnMouseMove(WPARAM WParam, int X, int Y)
{
	for (std::function<void(WPARAM, FMouseInputParameter&)>& Function : mInputActionFunctions[(int)EInputType::EIT_MouseMove])
	{
		mMouseInputParameter.SetParameters(X, Y, mLastX, mLastY);
		Function(WParam, mMouseInputParameter);
	}

	//if (WParam == MK_RBUTTON)
	//{
	//	static float Speed = 50.0f;
	//	// Make each pixel correspond to a quarter of a degree.
	//	float dx = XMConvertToRadians(0.25f * static_cast<float>(X - LastMousePos.x));
	//	float dy = XMConvertToRadians(0.25f * static_cast<float>(Y - LastMousePos.y));

	//	XMFLOAT3 Rotation = Camera->GetRotation();
	//	Camera->RotateY(dx * Speed);
	//	Camera->RotateX(dy * Speed);
	//}
	//LastMousePos.x = X;
	//LastMousePos.y = Y;
}

void FWindow::OnMouseWheel(WPARAM WParam)
{
	for (std::function<void(WPARAM, FMouseInputParameter&)>& Function : mInputActionFunctions[(int)EInputType::EIT_MouseWheel])
	{
		Function(WParam, mMouseInputParameter);
	}
}

void FWindow::OnKeyDown(WPARAM WParam)
{
	for (std::function<void(WPARAM, FMouseInputParameter&)>& Function : mInputActionFunctions[(int)EInputType::EIT_KeyDown])
	{
		Function(WParam, mMouseInputParameter);
	}
}

void FWindow::OnKeyUp(WPARAM WParam)
{
	for (std::function<void(WPARAM, FMouseInputParameter&)>& Function : mInputActionFunctions[(int)EInputType::EIT_KeyUp])
	{
		Function(WParam, mMouseInputParameter);
	}
}

void FWindow::SetInputAction(char Key, EInputType InputType, void (*Function)(FMouseInputParameter& MouseInputParameter))
{
	Key = tolower(Key);
	if ((int)InputType < (int)EInputType::EIT_KeyDown)
	{
		if (Key == 'r')
		{
			Key = tolower((char)MK_RBUTTON);
		}
		else if (Key == 'l')
		{
			Key = tolower((char)MK_LBUTTON);
		}
		else if (Key == 'm')
		{
			Key = tolower((char)MK_MBUTTON);
		}
		else
		{
			throw "Undefined MouseAction";
		}
	}
	mInputActionFunctions[(size_t)InputType].push_back([=](WPARAM WParam, FMouseInputParameter& MouseInputParameter)
		{
			if (tolower((char)WParam) == Key)
			{
				Function(MouseInputParameter);
			}
		}
	);
}
