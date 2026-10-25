#include <windows.h>
#include <gdiplus.h>
#include <iostream>
#include <string>
#pragma comment(lib, "Gdiplus.lib")

LRESULT CALLBACK WindowProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);


ULONG_PTR gdiplusToken = 0;

int main()
{
	using Gdiplus::Graphics;
	using Gdiplus::Pen;
	using Gdiplus::SolidBrush;


	Gdiplus::GdiplusStartupInput input;
	if (Gdiplus::GdiplusStartup(&gdiplusToken, &input, nullptr) != Gdiplus::Ok)
		throw std::runtime_error("GdiplusStartup Failed");

	WNDCLASSEX wc{ 0 };
	wc.cbSize = sizeof(wc);
	wc.hInstance = GetModuleHandleW(NULL);
	wc.lpszClassName = L"LoginWindowClass";
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hbrBackground = (HBRUSH)COLOR_WINDOW;
	wc.hCursor = LoadCursor(NULL, IDC_HAND);
	wc.hIcon = nullptr;
	wc.hIconSm = nullptr;
	wc.lpfnWndProc = WindowProc;
	RegisterClassExW(&wc);


	DEVMODE dm;
	dm.dmSize = sizeof(dm);
	if (EnumDisplaySettingsW(NULL, ENUM_CURRENT_SETTINGS, &dm) == 0)
		throw std::runtime_error("EnumDisplaySettingsW Failed");

	DWORD screenWidth = dm.dmPelsWidth;
	DWORD screenHeight = dm.dmPelsHeight;

	DWORD dwStyle = WS_POPUP;
	DWORD dwExStyle = NULL;
	HWND hMainWindow = CreateWindowExW(
		dwExStyle,
		L"LoginWindowClass",
		L"MainWindow",
		dwStyle,
		screenWidth / 8,
		screenHeight / 8,
		screenWidth / 3,
		screenHeight / 3,
		NULL,
		NULL,
		GetModuleHandleW(NULL),
		NULL);

	ShowWindow(hMainWindow, SW_NORMAL);
	UpdateWindow(hMainWindow);

	//消息循环
	MSG msg;
	while (GetMessage(&msg, hMainWindow, NULL, NULL))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	
	Gdiplus::GdiplusShutdown(gdiplusToken);
	return 0;
}


LRESULT OnWMPaint(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	
}


LRESULT CALLBACK WindowProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
		;
		case WM_DESTROY:
			PostQuitMessage(0);
			break;

		default:
			break;
	}
	return DefWindowProcW(hWnd, msg, wParam, lParam);
}



