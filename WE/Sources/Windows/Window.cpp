#include "Window.h"

#include <string>

FWindow::FWindow(const WCHAR* Title, int CmdShow, UINT Width, UINT Height, DWORD Style)
{
	Initialize(Title, CmdShow, Width, Height, Style);
}

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

bool FWindow::Initialize(const WCHAR* Title, int CmdShow, UINT Width, UINT Height, DWORD Style)
{
	static UINT WindowCount = 0;
	WCHAR ClassName[256];
	lstrcpy(ClassName, std::to_wstring(WindowCount++).c_str());
	// RegisterClass
	WNDCLASSEXW wcex;
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = GetHInstance();
	wcex.hIcon = LoadIcon(GetHInstance(), IDI_APPLICATION);
	wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = nullptr;
	wcex.lpszClassName = ClassName;
	wcex.hIconSm = LoadIcon(GetHInstance(), IDI_APPLICATION);
	RegisterClassExW(&wcex);

	// InitInstance
	hWnd = CreateWindowW(
		ClassName, Title,
		Style,
		CW_USEDEFAULT, CW_USEDEFAULT, Width, Height,
		nullptr, nullptr,
		GetHInstance(),
		this
	);

	if (!hWnd)
	{
		return false;
	}

	ShowWindow(hWnd, CmdShow);
	UpdateWindow(hWnd);
	return true;
}

LRESULT FWindow::InternalWndProc(HWND HandleWindow, UINT Message, WPARAM WParameter, LPARAM LParameter)
{
	switch (Message)
	{
		// �����찡 Ȱ��ȭ Ȥ�� ��Ȱ��ȭ �ɶ�
	case WM_ACTIVATE:
	{
		return 0;
	}
	// �������� ����� ����Ǿ�����
	case WM_SIZE:
	{
		return 0;
	}
	// �������� resize��(�����ڸ�)�� ���� �����Ҷ�
	case WM_ENTERSIZEMOVE:
	{
		return 0;
	}
	// �������� resize��(�����ڸ�)�� ���⸦ ������
	case WM_EXITSIZEMOVE:
	{
		return 0;
	}
	// ������ â �ݱ�
	case WM_DESTROY:
	{
		PostQuitMessage(0);
		return 0;
	}
	// �޴� �ٰ� Ȱ��ȭ �ɶ�
	case WM_MENUCHAR:
	{
		// ALT+Enter�� ��üȭ���� Ű�� ����, �Ҹ��� �ȳ��� ����.
		return MAKELRESULT(0, MNC_CLOSE);
	}
	// ������ �ִ�/�ּ� ũ�� ����
	case WM_GETMINMAXINFO:
	{
		((MINMAXINFO*)LParameter)->ptMinTrackSize.x = 200;
		((MINMAXINFO*)LParameter)->ptMinTrackSize.y = 200;
		return 0;
	}
	case WM_LBUTTONDOWN:
	case WM_MBUTTONDOWN:
	case WM_RBUTTONDOWN:
	case WM_LBUTTONUP:
	case WM_MBUTTONUP:
	case WM_RBUTTONUP:
	case WM_MOUSEMOVE:
	case WM_KEYDOWN:
	{
		switch (WParameter)
		{
		case 'W':
		{
			MessageBox(hWnd, L"TestTitle", L"TestCaption", 0);
			break;
		}
		default:
			break;
		}
	}
	case WM_PAINT:
	default:
		return DefWindowProc(HandleWindow, Message, WParameter, LParameter);
	}
	return 0;
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

