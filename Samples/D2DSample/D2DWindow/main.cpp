#include <windows.h>
#include <d2d1.h>
#include <stdexcept>
#include <iostream>
#include <wrl/client.h>
#include <winrt/windows.system.threading.h>
#pragma comment(lib, "d2d1.lib")


// 全局变量
HWND g_hMainWindow = nullptr;
HINSTANCE g_hInstance = nullptr;
const wchar_t* g_lpszClassName = L"D2DWindow";
const wchar_t* g_lpszWindowName = L"D2DWindow";

// Direct2D 对象
using Microsoft::WRL::ComPtr;
ComPtr<ID2D1Factory> g_pFactory;
ComPtr<ID2D1HwndRenderTarget> g_pHwndRenderTarget;
ComPtr<ID2D1SolidColorBrush> g_pSolidColorBrush;
ComPtr<ID2D1LinearGradientBrush> g_pLinearGradientBrush;
ComPtr<ID2D1RadialGradientBrush> g_pRadialGradialBrsh;

HRESULT CreateDeviceResources();
HRESULT CreateDeviceIndependentResources();
void DiscardDeviceResources();
void DiscardDeviceIndependentResources();

HRESULT OnRender();
void OnReSize(UINT uWidth, UINT uHeight);

LRESULT OnWMNcCreate(HWND hWnd, WPARAM wParam, LPARAM lParam);
LRESULT OnWMSize(HWND hWnd, WPARAM wParam, LPARAM lParam);
LRESULT OnWMCreate(HWND hWnd, WPARAM wParam, LPARAM lParam);
LRESULT OnWMDestroy(HWND hWnd, WPARAM wParam, LPARAM lParam);
LRESULT OnWMPaint(HWND hWnd, WPARAM wParam, LPARAM lParam);
LRESULT OnWMEraseBkgnd(HWND hWnd, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);



int main()
{
	g_hInstance = GetModuleHandleW(NULL);
	HRESULT hr = CreateDeviceIndependentResources();
	if (FAILED(hr))
		throw std::runtime_error("CreateDeviceIndependentResources Failed\n");

	WNDCLASSEX wc{ 0 };
	wc.cbSize = sizeof(wc);
	wc.lpfnWndProc = WindowProc;
	wc.hInstance = g_hInstance;
	wc.lpszClassName = g_lpszClassName;
	wc.hCursor = LoadCursorW(NULL, IDC_ARROW);
	wc.style = CS_HREDRAW | CS_VREDRAW;

	if (RegisterClassExW(&wc) == 0)
		throw std::runtime_error("RegisterClassExW Failed");

	DEVMODE dm;
	dm.dmSize = sizeof(dm);
	EnumDisplaySettings(NULL, ENUM_CURRENT_SETTINGS, &dm);
	DWORD dwScreenWidth = dm.dmPelsWidth;
	DWORD dwScreenHeight = dm.dmPelsHeight;

	DWORD dwStyle = WS_OVERLAPPEDWINDOW;
	DWORD dwExStyle = NULL;
	DWORD dwPosX = dwScreenWidth/8;
	DWORD dwPosY = dwScreenHeight/8;
	DWORD dwWidth = dwScreenWidth/2;
	DWORD dwHeight = dwScreenHeight/2;
	
	HWND hWnd = CreateWindowExW(dwExStyle,
		g_lpszClassName,
		g_lpszWindowName,
		dwStyle,
		dwPosX,
		dwPosY,
		dwWidth,
		dwHeight,
		nullptr,
		nullptr,
		g_hInstance,
		nullptr);

	if (hWnd == NULL)
		throw std::runtime_error("CreateWindowEx Failed\n");
	
	ShowWindow(g_hMainWindow, SW_NORMAL);
	UpdateWindow(g_hMainWindow);

	MSG msg;
	BOOL bRet = 0;
	while ((bRet = GetMessageW(&msg, nullptr, 0, 0)) != 0)
	{
		if (bRet == -1)
		{
			// Handle Error
			DWORD dwErrCode = GetLastError();
			return dwErrCode;
		}
		else {
			TranslateMessage(&msg);
			DispatchMessageW(&msg);
		}
	}

	return 0;
}


LRESULT OnWMNcCreate(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	g_hMainWindow = hWnd;
	return TRUE;
}

LRESULT OnWMEraseBkgnd(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	std::wcout << L"WM_ERASEBKGND \n";
	return 1;
}

LRESULT OnWMSize(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	if (wParam == SIZE_MINIMIZED)
		return 0;

	UINT uWidth = LOWORD(lParam);
	UINT uHeight = HIWORD(lParam);
	OnReSize(uWidth, uHeight);

	return 0;
}

LRESULT OnWMDestroy(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	PostQuitMessage(0);
	return 0;
}

LRESULT OnWMPaint(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	PAINTSTRUCT psPaint{};
	HDC hDc = BeginPaint(hWnd, &psPaint);
	HRESULT hr = OnRender();

	EndPaint(hWnd, &psPaint);
	if (FAILED(hr))
		std::wcout << L"Direct 2D 绘制失败\n";

	return 0;
}

LRESULT CALLBACK WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
		case WM_NCCREATE:
			return OnWMNcCreate(hWnd, wParam, lParam);

		case WM_PAINT:
			return OnWMPaint(hWnd, wParam, lParam);

		case WM_ERASEBKGND:
			return OnWMEraseBkgnd(hWnd, wParam, lParam); 

		case WM_SIZE:
			return OnWMSize(hWnd, wParam, lParam);

		case WM_DESTROY:
			return OnWMDestroy(hWnd, wParam, lParam);

		default:
			return DefWindowProcW(hWnd, uMsg, wParam, lParam);
	}
}

HRESULT CreateDeviceIndependentResources()
{
	HRESULT hr = D2D1CreateFactory(
		D2D1_FACTORY_TYPE_SINGLE_THREADED,
		g_pFactory.GetAddressOf());

	if (FAILED(hr))
		throw std::runtime_error("D2D1CreateFactory Failed");
	return hr;

}
HRESULT CreateDeviceResources()
{
	HRESULT hr = S_OK;
	if (g_pHwndRenderTarget != nullptr)
		return S_OK;
	if (g_pFactory == nullptr || g_hMainWindow == nullptr)
		return E_FAIL;
	RECT rcClient = {};
	if (!GetClientRect(g_hMainWindow, &rcClient))
		return HRESULT_FROM_WIN32(GetLastError());

	UINT uWidth = rcClient.right - rcClient.left;
	UINT uHeight = rcClient.bottom - rcClient.top;
	if (uWidth == 0 || uHeight == 0)
		return E_FAIL;

	//  CreateHwndRenderTarget
	hr = g_pFactory->CreateHwndRenderTarget(
		D2D1::RenderTargetProperties(),
		D2D1::HwndRenderTargetProperties(g_hMainWindow, D2D1::SizeU(uWidth, uHeight)),
		g_pHwndRenderTarget.GetAddressOf());

	if (SUCCEEDED(hr))
	{
		//CreateSolidColorBrush
		hr = g_pHwndRenderTarget->CreateSolidColorBrush(
			D2D1::ColorF(D2D1::ColorF::AntiqueWhite),
			g_pSolidColorBrush.GetAddressOf());

		if (FAILED(hr))
		{
			DiscardDeviceResources();
		}
	}
	return hr;
}


// Use D2D to Draw Window
HRESULT OnRender()
{
	HRESULT hr = CreateDeviceResources();
	if (FAILED(hr))
		return hr;

	g_pHwndRenderTarget->BeginDraw();

	// Clear Background
	g_pHwndRenderTarget->Clear(D2D1::ColorF(D2D1::ColorF::WhiteSmoke));

	// 使用SolidColorBrush绘制矩形
	g_pSolidColorBrush->SetColor(D2D1::ColorF(0x7B2CBF)); //高饱和紫
	g_pHwndRenderTarget->FillRectangle(
		D2D1::RectF(50.0f, 50.0f, 250.0f, 150.0f),
		g_pSolidColorBrush.Get());

	hr = g_pHwndRenderTarget->EndDraw();
	if (hr == D2DERR_RECREATE_TARGET)
	{
		DiscardDeviceResources();
		InvalidateRect(g_hMainWindow, nullptr, FALSE);
		hr = S_OK;
	}
	return hr;
}

void OnReSize(UINT uWidth, UINT uHeight)
{
	if (g_pHwndRenderTarget != nullptr)
	{
		HRESULT hr = g_pHwndRenderTarget->Resize(D2D1::SizeU(uWidth, uHeight));
		if (FAILED(hr))
			DiscardDeviceResources();
	}
	
	//请求重新绘制窗口
	if(g_hMainWindow != nullptr)
		InvalidateRect(g_hMainWindow, nullptr, FALSE);
}


void DiscardDeviceResources()
{
	// 先释放画刷, 再释放渲染目标
	g_pSolidColorBrush.Reset();
	g_pLinearGradientBrush.Reset();
	g_pRadialGradialBrsh.Reset();
	g_pHwndRenderTarget.Reset();
}

void DiscardDeviceIndependentResources()
{
	g_pFactory.Reset();
}