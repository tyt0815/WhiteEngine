#pragma once

#include "thread"
#include "Windows/WindowsUtil.h"
#include "Common/Common.h"
#include "Common/Timer.h"

class FWindow
{
public:
	FWindow(const WCHAR* Title = L"DefaultTitle",
		int CmdShow = 10, UINT Width = 1920, UINT Height = 1080,
		DWORD Style = WS_OVERLAPPED | WS_SYSMENU
	);
	std::wstring GetTitle();
	std::wstring GetClass();
	RECT GetClientRect();
	UINT GetClientWidth() { return this->GetClientRect().right - this->GetClientRect().left; }
	UINT GetClientHeight() { return this->GetClientRect().bottom - this->GetClientRect().top; }
	inline static HINSTANCE GetHInstance() { return GetModuleHandle(nullptr); }
	inline HWND GetHWnd() { return hWnd; }
	inline void SetTitle(const WCHAR* WString) { SetWindowText(hWnd, WString); }
	virtual bool Initialize(const WCHAR* Title = L"DefaultTitle",
		int CmdShow = 10, UINT Width = 1920, UINT Height = 1080,
		DWORD Style = WS_OVERLAPPED | WS_SYSMENU);
	virtual int Execute();
	static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
	virtual LRESULT InternalWndProc(HWND HandleWindow, UINT Message, WPARAM WParameter, LPARAM LParameter);

protected:
	UTimer MainTimer;
	bool bAppPaused = false;
	void CalculateFrameStats();
	virtual void Event() {}

private:

	HWND hWnd;
};