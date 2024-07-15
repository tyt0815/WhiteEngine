#pragma once

#include <string>
#include "WindowsHeaders.h"
#include "Utility/Timer.h"

class FWindow
{
public:
	FWindow() {}
	std::wstring GetTitle();
	std::wstring GetClass();
	RECT GetClientRect();
	UINT GetClientWidth() { return this->GetClientRect().right - this->GetClientRect().left; }
	UINT GetClientHeight() { return this->GetClientRect().bottom - this->GetClientRect().top; }
	inline float GetAspectRatio() { return static_cast<float>(GetClientWidth()) / GetClientHeight(); }
	inline static HINSTANCE GetHInstance() { return GetModuleHandle(nullptr); }
	inline HWND GetHWnd() { return hWnd; }
	inline void SetTitle(const WCHAR* WString) { SetWindowText(hWnd, WString); }
	virtual bool Initialize(
		const WCHAR* Title = L"DefaultTitle",
		int CmdShow = SW_SHOW,
		UINT Width = 800, UINT Height = 600,
		DWORD Style = WS_OVERLAPPEDWINDOW
	);
	int Execute();
	static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
	virtual LRESULT InternalWndProc(HWND HandleWindow, UINT Message, WPARAM WParameter, LPARAM LParameter);

protected:
	UTimer MainTimer;
	bool bAppPaused = false;
	void CalculateFrameStats();
	virtual void ProcessEvent() {}
	virtual void ResizeWindow() {}
	virtual void ClickMouse(WPARAM Button, int X, int Y);
	virtual void ReleaseMouse(WPARAM Button, int X, int Y);
	virtual void MoveMouse(WPARAM Button, int X, int Y);
	virtual void PressKey(WPARAM Key) {}
	inline bool IsMouseClicked(WPARAM Button, UINT16 ButtonMask) { return (Button & ButtonMask) != 0; }

	int ClickedX = 0;
	int ClickedY = 0;

private:
	HWND hWnd = nullptr;

};