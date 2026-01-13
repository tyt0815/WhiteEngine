#pragma once

#include <windows.h>
#include <string>
#include <functional>
#include <vector> 
#include <any>

#include "Utility/Class.h"
#include "GameFramework/InputSystem/InputSystem.h"

extern HINSTANCE g_hInst;

class FWindow : FNoncopyable
{
public:
	FWindow(const std::wstring ClassName, const std::wstring WindowName, UINT Width, UINT Height);
	LRESULT WindowProcedure(HWND WindowHandle, UINT Message, WPARAM WParam, LPARAM LParam);

private:
	void UpdateWindowSize();
	void Resize();
	void OnMouseDown(WPARAM WParam, int X, int Y);
	void OnKeyDown(WPARAM WParam);
	HWND mWindowHandle = nullptr;
	std::wstring mClassName;
	std::wstring mWindowName;
	std::vector<std::function<void()>> mResizeCallbackFunctions;
	

	UINT mWidth = 0;
	UINT mHeight = 0;
	UINT mLastX = 0;
	UINT mLastY = 0;

	bool bPaused = false;
	bool bResized = false;
	bool bMinimized = false;
	bool bMaximized = false;
	bool bCaptured = false;

public:
	inline HWND	GetWindowHandle() const { return mWindowHandle; }
	inline std::wstring GetWindowName() const { return mWindowName; }
	inline UINT GetWidth() const { return mWidth; }
	inline UINT GetHeight() const { return mHeight; }
	inline bool IsPaused() const { return bPaused; }
	void SetResizeCallbackFunction(std::function<void()> Function)
	{
		mResizeCallbackFunctions.push_back(Function);
	}
	inline bool IsCaptured() const 
	{ 
		return bCaptured; 
	}
};

extern FWindow gMainWindow;

inline FWindow* GetMainWindowPtr()
{
	return &gMainWindow;
}

inline HWND GetHWND()
{
	return gMainWindow.GetWindowHandle();
}