#include "Window.h"

#include <string>

std::wstring FWindow::GetTitle()
{
	WCHAR Title[256];
	GetWindowText(hWnd, Title, int(256));
	return std::wstring(Title);
}

std::wstring FWindow::GetClass()
{
	WCHAR ClassName[256];
	GetClassName(hWnd, ClassName, 256);
	return std::wstring(ClassName);
}

RECT FWindow::GetClientRect()
{
	RECT Rect;
	::GetClientRect(hWnd, &Rect);
	return Rect;
}

bool FWindow::Initialize(
	const WCHAR* Title,
	int CmdShow,
	UINT Width, UINT Height,
	DWORD Style
)
{
	static UINT WindowCount = 0;
	WCHAR ClassName[256];
	lstrcpy(ClassName, std::to_wstring(WindowCount++).c_str());
	// RegisterClass
	WNDCLASS wc;
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = WndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = GetHInstance();
	wc.hIcon = LoadIcon(0, IDI_APPLICATION);
	wc.hCursor = LoadCursor(0, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)GetStockObject(NULL_BRUSH);
	wc.lpszMenuName = 0;
	wc.lpszClassName = ClassName;
	if (!RegisterClass(&wc))
	{
		MessageBox(0, L"RegisterClass Failed.", 0, 0);
		return false;
	}

	RECT R = { 0, 0, static_cast<LONG>(Width), static_cast<LONG>(Height) };
	AdjustWindowRect(&R, Style, false);
	Width = R.right - R.left;
	Height = R.bottom - R.top;

	hWnd = CreateWindow(
		ClassName,
		Title,
		Style,
		CW_USEDEFAULT, CW_USEDEFAULT,
		Width, Height,
		0, 0, GetHInstance(), 0
	);

	if (!hWnd)
	{
		return false;
	}

	ShowWindow(hWnd, CmdShow);
	UpdateWindow(hWnd);
	return true;
}

int FWindow::Execute()
{
	MSG msg = { 0 };

	MainTimer.Reset();

	while (msg.message != WM_QUIT)
	{
		// If there are Window messages then process them.
		if (PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		// Otherwise, do animation/game stuff.
		else
		{
			MainTimer.Tick();

			if (!bAppPaused)
			{
				CalculateFrameStats();
				ProcessEvent();
			}
		}
	}

	return (int)msg.wParam;
}

LRESULT FWindow::InternalWndProc(HWND HandleWindow, UINT Message, WPARAM WParameter, LPARAM LParameter)
{
	switch (Message)
	{
		// 윈도우가 활성화 혹은 비활성화 될때
	case WM_ACTIVATE:
	{
		return 0;
	}
	// 윈도우의 사이즈가 변경되었을때
	case WM_SIZE:
	{
		ResizeWindow();
		return 0;
	}
	// 윈도우의 resize바(가장자리)를 끌기 시작할때
	case WM_ENTERSIZEMOVE:
	{
		return 0;
	}
	// 윈도우의 resize바(가장자리)를 끌기를 끝낼때
	case WM_EXITSIZEMOVE:
	{
		return 0;
	}
	// 윈도우 창 닫기
	case WM_DESTROY:
	{
		PostQuitMessage(0);
		return 0;
	}
	// 메뉴 바가 활성화 될때
	case WM_MENUCHAR:
	{
		// ALT+Enter로 전체화면을 키고 끌때, 소리가 안나게 해줌.
		return MAKELRESULT(0, MNC_CLOSE);
	}
	// 윈도우 최대/최소 크기 설정
	case WM_GETMINMAXINFO:
	{
		((MINMAXINFO*)LParameter)->ptMinTrackSize.x = 200;
		((MINMAXINFO*)LParameter)->ptMinTrackSize.y = 200;
		return 0;
	}
	case WM_LBUTTONDOWN:
	case WM_MBUTTONDOWN:
	case WM_RBUTTONDOWN:
		ClickMouse(WParameter, GET_X_LPARAM(LParameter), GET_Y_LPARAM(LParameter));
		break;
	case WM_LBUTTONUP:
	case WM_MBUTTONUP:
	case WM_RBUTTONUP:
		ReleaseMouse(WParameter, GET_X_LPARAM(LParameter), GET_Y_LPARAM(LParameter));
		break;
	case WM_MOUSEMOVE:
		MoveMouse(WParameter, GET_X_LPARAM(LParameter), GET_Y_LPARAM(LParameter));
		break;
	case WM_KEYDOWN:
		PressKey(WParameter);
		break;
	case WM_PAINT:
	default:
		return DefWindowProc(HandleWindow, Message, WParameter, LParameter);
	}
	return 0;
}

void FWindow::CalculateFrameStats()
{
	// Code computes the average frames per second, and also the 
	// average time it takes to render one frame.  These stats 
	// are appended to the window caption bar.

	static int frameCnt = 0;
	static double timeElapsed = 0.0f;

	frameCnt++;

	// Compute averages over one second period.
	if ((MainTimer.GetTotalTime() - timeElapsed) >= 1.0f)
	{
		float fps = (float)frameCnt; // fps = frameCnt / 1
		float mspf = 1000.0f / fps;

		std::wstring fpsStr = std::to_wstring(fps);
		std::wstring mspfStr = std::to_wstring(mspf);
		static std::wstring WindowTitle = GetTitle();
		std::wstring windowText = WindowTitle +
			L"    fps: " + fpsStr +
			L"   mspf: " + mspfStr +
			L"TotalTime: " + std::to_wstring(MainTimer.GetTotalTime()) +
			L"ElapsedTime:" + std::to_wstring(timeElapsed);


		SetWindowText(GetHWnd(), windowText.c_str());

		// Reset for next average.
		frameCnt = 0;
		timeElapsed += 1.0f;
	}
}

void FWindow::ClickMouse(WPARAM Button, int X, int Y)
{
	ClickedX = X;
	ClickedY = Y;

	SetCapture(hWnd);
}

void FWindow::ReleaseMouse(WPARAM Button, int X, int Y)
{
	ReleaseCapture();
}

void FWindow::MoveMouse(WPARAM Button, int X, int Y)
{

}

LRESULT FWindow::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	FWindow* pState;
	if (message == WM_CREATE)
	{
		CREATESTRUCT* pCreate = reinterpret_cast<CREATESTRUCT*>(lParam);
		pState = reinterpret_cast<FWindow*>(pCreate->lpCreateParams);
		SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)pState);
	}
	else
	{
		LONG_PTR ptr = GetWindowLongPtr(hWnd, GWLP_USERDATA);
		pState = reinterpret_cast<FWindow*>(ptr);
	}
	if (pState)
	{
		pState->InternalWndProc(hWnd, message, wParam, lParam);
	}

	return DefWindowProc(hWnd, message, wParam, lParam);
}

