#include <windows.h>
#include <gdiplus.h>
#include <iostream>
#include <string>
#pragma comment(lib, "Gdiplus.lib")

LRESULT CALLBACK WindowProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
ULONG_PTR gdiplusToken = 0;

int main()
{
	Gdiplus::GdiplusStartupInput input;
	if (Gdiplus::GdiplusStartup(&gdiplusToken, &input, nullptr) != Gdiplus::Ok)
		throw std::runtime_error("GdiplusStartup Failed");

	WNDCLASSEX wc{ 0 };
	wc.cbSize = sizeof(wc);
	wc.hbrBackground = nullptr;
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

	DWORD dwStyle = WS_OVERLAPPEDWINDOW;
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


/*LRESULT OnWMPaint(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	PAINTSTRUCT ps{};
	HDC hDc = BeginPaint(hWnd, &ps);
	RECT rc{};
	GetClientRect(hWnd, &rc);

	{
		Gdiplus::Graphics graphics(hDc);
		Gdiplus::LinearGradientBrush brush(
											Gdiplus::Point(0,0),
											Gdiplus::Point(rc.right, rc.bottom),
											Gdiplus::Color(255, 210, 235, 255),
											Gdiplus::Color(255, 255, 220, 235));

		graphics.FillRectangle(
								&brush,
								0,
								0,
								rc.right,
								rc.bottom);
	}

	EndPaint(hWnd, &ps);
	return 0;
}*/



LRESULT OnWMPaint(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	PAINTSTRUCT ps{};
	HDC hdc = BeginPaint(hWnd, &ps);

	RECT rc{};
	GetClientRect(hWnd, &rc);

	const int width = rc.right - rc.left;
	const int height = rc.bottom - rc.top;

	if (width > 0 && height > 0)
	{
		// 1. 创建内存设备上下文
		HDC hMemDC = CreateCompatibleDC(hdc);

		if (hMemDC)
		{
			// 2. 创建与窗口兼容的内存位图
			HBITMAP hBitmap =
				CreateCompatibleBitmap(hdc, width, height);

			if (hBitmap)
			{
				HGDIOBJ hOldBitmap = SelectObject(hMemDC, hBitmap);

				// 3. 先在内存位图中绘制渐变
				{
					Gdiplus::Graphics graphics(hMemDC);

					graphics.SetSmoothingMode(
						Gdiplus::SmoothingModeAntiAlias);

					Gdiplus::LinearGradientBrush brush(
						Gdiplus::Point(0, 0),
						Gdiplus::Point(width, height),
						Gdiplus::Color(255, 210, 235, 255),
						Gdiplus::Color(255, 255, 220, 235));

					graphics.FillRectangle(
						&brush, 0, 0, width, height);
				}

				// 4. 将绘制完成的位图复制到窗口
				BitBlt(
					hdc,
					0, 0,
					width, height,
					hMemDC,
					0, 0,
					SRCCOPY);

				// 5. 恢复并释放 GDI 对象
				SelectObject(hMemDC, hOldBitmap);
				DeleteObject(hBitmap);
			}

			DeleteDC(hMemDC);
		}
	}

	EndPaint(hWnd, &ps);
	return 0;
}


LRESULT CALLBACK WindowProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
		case WM_PAINT:
			return OnWMPaint(hWnd, wParam, lParam);

		case WM_ERASEBKGND:
			return 1;

		case WM_DESTROY:
			PostQuitMessage(0);
			break;

		default:
			break;
	}
	return DefWindowProcW(hWnd, msg, wParam, lParam);
}