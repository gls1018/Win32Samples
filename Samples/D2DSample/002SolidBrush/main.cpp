
#include <windows.h>
#include <windowsx.h>
#include <commdlg.h>
#include <d2d1.h>
#include <wincodec.h>
#include <wrl/client.h>
#include <string>
#include <algorithm>
#include <cmath>
#include <shellscalingapi.h>
#pragma comment(lib, "d2d1.lib")
#pragma comment(lib, "windowscodecs.lib")
#pragma comment(lib, "ole32.lib")
#pragma comment(lib, "comdlg32.lib")
#pragma comment(lib, "Shcore.lib")




using Microsoft::WRL::ComPtr;

constexpr UINT IDM_FILE_OPEN = 1001;
constexpr UINT IDM_FILE_EXIT = 1002;

HINSTANCE g_hInstance = nullptr;
HWND g_hWnd = nullptr;
ComPtr<ID2D1Factory> g_pD2DFactory;
ComPtr<IWICImagingFactory> g_pWICFactory;
ComPtr<ID2D1HwndRenderTarget> g_pRenderTarget;
ComPtr<ID2D1Bitmap> g_pBitmap;
ComPtr<IWICBitmapSource> g_pWICBitmapSource;
UINT g_uImageWidth = 0;
UINT g_uImageHeight = 0;
std::wstring g_strImagePath;

void DiscardDeviceResources()
{
    g_pBitmap.Reset();
    g_pRenderTarget.Reset();
}

HRESULT CreateDeviceIndependentResources()
{
    HRESULT hr = D2D1CreateFactory(
        D2D1_FACTORY_TYPE_SINGLE_THREADED,
        g_pD2DFactory.ReleaseAndGetAddressOf());
    if (FAILED(hr)) return hr;

    hr = CoCreateInstance(CLSID_WICImagingFactory, nullptr,
        CLSCTX_INPROC_SERVER,
        IID_PPV_ARGS(g_pWICFactory.ReleaseAndGetAddressOf()));
    return hr;
}

HRESULT CreateDeviceResources()
{
    if (g_pRenderTarget) return S_OK;
    if (!g_pD2DFactory || !g_hWnd) return E_FAIL;

    RECT rcClient = {};
    if (!GetClientRect(g_hWnd, &rcClient))
        return HRESULT_FROM_WIN32(GetLastError());

    UINT uWidth = static_cast<UINT>(rcClient.right - rcClient.left);
    UINT uHeight = static_cast<UINT>(rcClient.bottom - rcClient.top);
    // HWND target requires nonzero dimensions during creation.
    uWidth = (std::max)(uWidth, 1u);
    uHeight = (std::max)(uHeight, 1u);

    HRESULT hr = g_pD2DFactory->CreateHwndRenderTarget(
        D2D1::RenderTargetProperties(),
        D2D1::HwndRenderTargetProperties(g_hWnd, D2D1::SizeU(uWidth, uHeight)),
        g_pRenderTarget.ReleaseAndGetAddressOf());
    if (FAILED(hr)) return hr;

    if (g_pWICBitmapSource)
    {
        hr = g_pRenderTarget->CreateBitmapFromWicBitmap(
            g_pWICBitmapSource.Get(), nullptr,
            g_pBitmap.ReleaseAndGetAddressOf());
        if (FAILED(hr)) DiscardDeviceResources();
    }
    return hr;
}

HRESULT LoadImageFile(const wchar_t* pszFileName)
{
    if (!g_pWICFactory || !pszFileName) return E_INVALIDARG;

    // Decode into local objects first, so failure keeps the current image intact.
    ComPtr<IWICBitmapDecoder> pDecoder;
    ComPtr<IWICBitmapFrameDecode> pFrame;
    ComPtr<IWICFormatConverter> pConverter;
    ComPtr<IWICBitmap> pCachedBitmap;
    UINT uWidth = 0, uHeight = 0;

    HRESULT hr = g_pWICFactory->CreateDecoderFromFilename(
        pszFileName, nullptr, GENERIC_READ, WICDecodeMetadataCacheOnLoad,
        pDecoder.GetAddressOf());
    if (SUCCEEDED(hr)) hr = pDecoder->GetFrame(0, pFrame.GetAddressOf());
    if (SUCCEEDED(hr)) hr = pFrame->GetSize(&uWidth, &uHeight);
    if (SUCCEEDED(hr)) hr = g_pWICFactory->CreateFormatConverter(pConverter.GetAddressOf());
    if (SUCCEEDED(hr))
    {
        hr = pConverter->Initialize(pFrame.Get(), GUID_WICPixelFormat32bppPBGRA,
            WICBitmapDitherTypeNone, nullptr, 0.0, WICBitmapPaletteTypeCustom);
    }
    // Cache converted pixels independently of decoder/file lifetime.
    if (SUCCEEDED(hr))
    {
        hr = g_pWICFactory->CreateBitmapFromSource(
            pConverter.Get(), WICBitmapCacheOnLoad, pCachedBitmap.GetAddressOf());
    }

    // If a render target exists, create the new D2D bitmap before committing.
    ComPtr<ID2D1Bitmap> pNewBitmap;
    if (SUCCEEDED(hr) && g_pRenderTarget)
    {
        hr = g_pRenderTarget->CreateBitmapFromWicBitmap(
            pCachedBitmap.Get(), nullptr, pNewBitmap.GetAddressOf());
    }
    if (FAILED(hr)) return hr;

    g_pWICBitmapSource = pCachedBitmap;
    g_pBitmap = pNewBitmap;
    g_uImageWidth = uWidth;
    g_uImageHeight = uHeight;
    g_strImagePath = pszFileName;
    if (g_hWnd) InvalidateRect(g_hWnd, nullptr, FALSE);
    return S_OK;
}

HRESULT OnRender()
{
    HRESULT hr = CreateDeviceResources();
    if (FAILED(hr)) return hr;

    g_pRenderTarget->BeginDraw();
    g_pRenderTarget->SetTransform(D2D1::Matrix3x2F::Identity());
    g_pRenderTarget->Clear(D2D1::ColorF(0.10f, 0.13f, 0.18f));

    if (g_pBitmap && g_uImageWidth && g_uImageHeight)
    {
        D2D1_SIZE_F sizeClient = g_pRenderTarget->GetSize();
        D2D1_SIZE_F sizeBitmap = g_pBitmap->GetSize();
        if (sizeClient.width > 0 && sizeClient.height > 0 &&
            sizeBitmap.width > 0 && sizeBitmap.height > 0)
        {
            FLOAT fScale = (std::min)(
                sizeClient.width / sizeBitmap.width,
                sizeClient.height / sizeBitmap.height);
            // Fit to window; never upscale small images in this lesson.
            fScale = (std::min)(fScale, 1.0f);
            FLOAT fWidth = sizeBitmap.width * fScale;
            FLOAT fHeight = sizeBitmap.height * fScale;
            FLOAT fLeft = (sizeClient.width - fWidth) * 0.5f;
            FLOAT fTop = (sizeClient.height - fHeight) * 0.5f;
            D2D1_RECT_F rcDest = D2D1::RectF(
                fLeft, fTop, fLeft + fWidth, fTop + fHeight);
            g_pRenderTarget->DrawBitmap(g_pBitmap.Get(), rcDest, 1.0f,
                D2D1_BITMAP_INTERPOLATION_MODE_LINEAR);
        }
    }

    hr = g_pRenderTarget->EndDraw();
    if (hr == D2DERR_RECREATE_TARGET)
    {
        DiscardDeviceResources();
        if (g_hWnd) InvalidateRect(g_hWnd, nullptr, FALSE);
        return S_OK;
    }
    return hr;
}

void OnResize(UINT uWidth, UINT uHeight)
{
    if (g_pRenderTarget && uWidth && uHeight)
    {
        HRESULT hr = g_pRenderTarget->Resize(D2D1::SizeU(uWidth, uHeight));
        if (FAILED(hr)) DiscardDeviceResources();
    }
    if (g_hWnd) InvalidateRect(g_hWnd, nullptr, FALSE);
}

void OpenImageDialog(HWND hWnd)
{
    wchar_t szFileName[32768] = {};
    OPENFILENAMEW ofn = {};
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = hWnd;
    ofn.lpstrFilter = L"图片文件\0*.png;*.jpg;*.jpeg;*.bmp;*.gif;*.tif;*.tiff\0所有文件\0*.*\0";
    ofn.lpstrFile = szFileName;
    ofn.nMaxFile = ARRAYSIZE(szFileName);
    ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_EXPLORER;

    if (GetOpenFileNameW(&ofn))
    {
        HRESULT hr = LoadImageFile(szFileName);
        if (FAILED(hr))
        {
            wchar_t szMessage[160] = {};
            wsprintfW(szMessage, L"无法加载图片。\nHRESULT: 0x%08X", static_cast<UINT>(hr));
            MessageBoxW(hWnd, szMessage, L"图片加载失败", MB_OK | MB_ICONERROR);
        }
    }
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
        case WM_COMMAND:
            switch (LOWORD(wParam))
            {
                case IDM_FILE_OPEN: OpenImageDialog(hWnd); return 0;
                case IDM_FILE_EXIT: DestroyWindow(hWnd); return 0;
            }
            break;
        case WM_SIZE:
            OnResize(LOWORD(lParam), HIWORD(lParam));
            return 0;
        case WM_PAINT:
        {
            PAINTSTRUCT psPaint = {};
            BeginPaint(hWnd, &psPaint);
            HRESULT hr = OnRender();
            EndPaint(hWnd, &psPaint);
            if (FAILED(hr)) OutputDebugStringW(L"Direct2D OnRender failed.\n");
            return 0;
        }
        case WM_DISPLAYCHANGE:
            InvalidateRect(hWnd, nullptr, FALSE);
            return 0;
        case WM_ERASEBKGND:
            return 1; // Direct2D clears the full client area.
        case WM_DESTROY:
            DiscardDeviceResources();
            g_hWnd = nullptr;
            PostQuitMessage(0);
            return 0;
    }
    return DefWindowProcW(hWnd, uMsg, wParam, lParam);
}

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, PWSTR, int nCmdShow)
{
    SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);

    g_hInstance = hInstance;
    HRESULT hr = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
    if (FAILED(hr))
    {
        MessageBoxW(nullptr, L"COM 初始化失败。", L"错误", MB_OK | MB_ICONERROR);
        return -1;
    }

    int nExitCode = -1;
    do
    {
        hr = CreateDeviceIndependentResources();
        if (FAILED(hr)) break;

        const wchar_t* pszClassName = L"Direct2DWICViewerLesson02";
        WNDCLASSEXW wcWindow = {};
        wcWindow.cbSize = sizeof(wcWindow);
        wcWindow.style = CS_HREDRAW | CS_VREDRAW;
        wcWindow.lpfnWndProc = WndProc;
        wcWindow.hInstance = hInstance;
        wcWindow.hCursor = LoadCursorW(nullptr, IDC_ARROW);
        wcWindow.lpszClassName = pszClassName;
        if (!RegisterClassExW(&wcWindow)) break;

        HMENU hMenu = CreateMenu();
        HMENU hFileMenu = CreatePopupMenu();
        if (!hMenu || !hFileMenu)
        {
            if (hFileMenu) DestroyMenu(hFileMenu);
            if (hMenu) DestroyMenu(hMenu);
            break;
        }
        AppendMenuW(hFileMenu, MF_STRING, IDM_FILE_OPEN, L"打开图片(&O)...");
        AppendMenuW(hFileMenu, MF_SEPARATOR, 0, nullptr);
        AppendMenuW(hFileMenu, MF_STRING, IDM_FILE_EXIT, L"退出(&X)");
        AppendMenuW(hMenu, MF_POPUP, reinterpret_cast<UINT_PTR>(hFileMenu), L"文件(&F)");

        g_hWnd = CreateWindowExW(0, pszClassName, L"Direct2D 图片查看器 - WIC 第 2 课",
            WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, 900, 650,
            nullptr, hMenu, hInstance, nullptr);
        if (!g_hWnd)
        {
            DestroyMenu(hMenu); // Window did not take ownership.
            break;
        }
        ShowWindow(g_hWnd, nCmdShow);
        UpdateWindow(g_hWnd);

        MSG msg = {};
        BOOL bResult = FALSE;
        while ((bResult = GetMessageW(&msg, nullptr, 0, 0)) > 0)
        {
            TranslateMessage(&msg);
            DispatchMessageW(&msg);
        }
        nExitCode = bResult == -1 ? -1 : static_cast<int>(msg.wParam);
    } while (false);

    DiscardDeviceResources();
    g_pWICBitmapSource.Reset();
    g_pWICFactory.Reset();
    g_pD2DFactory.Reset();
    CoUninitialize();
    return nExitCode;
}
