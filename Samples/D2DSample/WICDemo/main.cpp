#include <windows.h>
#include <windowsx.h>
#include <wrl/client.h>
#include <string>
#include <algorithm>
#include <cmath> 
#include <d2d1.h>

#pragma comment(lib, "d2d1.lib")
#pragma comment(lib, "windowscodecs.lib")
#pragma comment(lib, "ole32.lib")
#pragma comment(lib, "comdlg32.lib")
#pragma comment(lib, "Shcore.lib")


HINSTANCE g_hInstance = nullptr;
HWND g_hMainWindow = nullptr;
const wchar_t* lpszClassName = L"D2DWindow";
const wchar_t* lpszWindowName = L"D2DWindow";
const UINT IDM_FILE_OPEN = 0x1001;
const UINT IDM_FILE_EXIT = 0x1002;


using Microsoft::WRL::ComPtr;
ComPtr<ID2D1Factory> g_pD2D1Factory;
LRESULT CALLBACK WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);


int main()
{
	SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);

	do {
		WNDCLASSEXW wc{ };
		wc.cbSize = sizeof(wc);
		wc.hInstance = GetModuleHandleW(NULL);
		wc.lpszClassName = lpszClassName;
		wc.lpfnWndProc = WindowProc;
		wc.style = CS_HREDRAW | CS_VREDRAW;
		wc.hCursor = LoadCursorW(nullptr, IDC_ARROW);

		if (RegisterClassExW(&wc) == 0)
		{
			int Errcode = GetLastError();
			break;
		}
			

		HMENU hMenu = CreateMenu();
		HMENU hFileMenu = CreatePopupMenu();
		if (!hMenu || !hFileMenu)
		{
			if (hMenu) DestroyMenu(hMenu);
			if (hFileMenu) DestroyMenu(hFileMenu);
			break;
		}

		AppendMenuW(hFileMenu, MF_STRING, IDM_FILE_OPEN, L"打开图片(&O)...");
		AppendMenuW(hFileMenu, MF_SEPARATOR, 0, nullptr);
		AppendMenuW(hFileMenu, MF_STRING, IDM_FILE_EXIT, L"退出(&X)");
		AppendMenuW(hMenu, MF_POPUP, (UINT_PTR)hFileMenu, L"文件");

		// 获取屏幕分辨率
		DEVMODE dm;
		dm.dmSize = sizeof(dm);
		EnumDisplaySettingsW(NULL, ENUM_CURRENT_SETTINGS, &dm);
		DWORD screenWidth = dm.dmPelsWidth;
		DWORD screenHeight = dm.dmPelsHeight;

		DWORD dwStyle = WS_OVERLAPPEDWINDOW;
		DWORD dwExStyle = NULL;
		HWND hWnd = CreateWindowExW(dwExStyle, 
			lpszClassName, 
			lpszWindowName, 
			dwStyle,
			screenWidth / 8, screenHeight / 8, screenWidth / 3, screenHeight / 3, 
			NULL, 
			hMenu, 
			GetModuleHandleW(NULL), 
			NULL);

		if (hWnd == NULL)
		{
			DestroyMenu(hMenu);
			break;
		}
		ShowWindow(hWnd, SW_NORMAL);
		UpdateWindow(hWnd);

		MSG msg = {};
		while (GetMessageW(&msg, NULL, 0, 0))
		{
			TranslateMessage(&msg);
			DispatchMessageW(&msg);
		}
	} while (false);
	return 0;
}


LRESULT OnWMDestroy(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	PostQuitMessage(0);
	return 0;
}


LRESULT CALLBACK WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
		case WM_DESTROY:
			return OnWMDestroy(hWnd, wParam, lParam);

		default:
			return DefWindowProcW(hWnd, uMsg, wParam, lParam);
	}
}