#pragma once

#include "thread"
#include "Windows/WindowsUtil.h"
#include "Common/Common.h"

class FWindow
{
public:
	FWindow(const WCHAR* Title = L"DefaultTitle",
		int CmdShow = 10, UINT Width = 1920, UINT Height = 1080,
		DWORD Style = WS_OVERLAPPED | WS_SYSMENU
	);
	std::wstring GetTitle();
	std::wstring GetClass();
	inline void SetTitle(const WCHAR* WString) { SetWindowText(hWnd, WString); }
	inline HWND GetHWnd() { return hWnd; }
	virtual bool Initialize(const WCHAR* Title = L"DefaultTitle",
		int CmdShow = 10, UINT Width = 1920, UINT Height = 1080,
		DWORD Style = WS_OVERLAPPED | WS_SYSMENU);
	virtual LRESULT InternalWndProc(HWND HandleWindow, UINT Message, WPARAM WParameter, LPARAM LParameter);

private:
	virtual void Event() {}

	HWND hWnd;

	std::thread WindowThread;


public:
	inline static HINSTANCE GetHInstance() { return GetModuleHandle(nullptr); }
	static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
};