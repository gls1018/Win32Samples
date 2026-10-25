
#include <windows.h>
#include <d2d1.h>

#pragma comment(lib, "d2d1.lib")

// ============================================================
// 全局变量
// ============================================================

HINSTANCE g_hInstance = nullptr;
HWND g_hWnd = nullptr;

ID2D1Factory* g_pD2DFactory = nullptr;
ID2D1HwndRenderTarget* g_pRenderTarget = nullptr;
ID2D1SolidColorBrush* g_pBrush = nullptr;

// ============================================================
// 函数声明
// ============================================================

HRESULT CreateDeviceIndependentResources();
HRESULT CreateDeviceResources();
void DiscardDeviceResources();
void DiscardDeviceIndependentResources();

HRESULT OnRender();
void OnResize(UINT uWidth, UINT uHeight);

LRESULT CALLBACK WndProc(
    HWND hWnd,
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam
);

// ============================================================
// 创建设备无关资源
// ============================================================

HRESULT CreateDeviceIndependentResources()
{
    HRESULT hr = D2D1CreateFactory(
        D2D1_FACTORY_TYPE_SINGLE_THREADED,
        &g_pD2DFactory
    );

    return hr;
}

// ============================================================
// 创建设备相关资源
// ============================================================

HRESULT CreateDeviceResources()
{
    HRESULT hr = S_OK;

    if (g_pRenderTarget == nullptr)
    {
        RECT rcClient = {};
        GetClientRect(g_hWnd, &rcClient);

        UINT uWidth = rcClient.right - rcClient.left;
        UINT uHeight = rcClient.bottom - rcClient.top;

        hr = g_pD2DFactory->CreateHwndRenderTarget(
            D2D1::RenderTargetProperties(),
            D2D1::HwndRenderTargetProperties(
                g_hWnd,
                D2D1::SizeU(uWidth, uHeight)
            ),
            &g_pRenderTarget
        );

        if (SUCCEEDED(hr))
        {
            hr = g_pRenderTarget->CreateSolidColorBrush(
                D2D1::ColorF(D2D1::ColorF::White),
                &g_pBrush
            );
        }

        if (FAILED(hr))
        {
            DiscardDeviceResources();
        }
    }

    return hr;
}

// ============================================================
// 释放设备相关资源
// ============================================================

void DiscardDeviceResources()
{
    if (g_pBrush != nullptr)
    {
        g_pBrush->Release();
        g_pBrush = nullptr;
    }

    if (g_pRenderTarget != nullptr)
    {
        g_pRenderTarget->Release();
        g_pRenderTarget = nullptr;
    }
}

// ============================================================
// 释放设备无关资源
// ============================================================

void DiscardDeviceIndependentResources()
{
    if (g_pD2DFactory != nullptr)
    {
        g_pD2DFactory->Release();
        g_pD2DFactory = nullptr;
    }
}

// ============================================================
// 绘制窗口
// ============================================================

HRESULT OnRender()
{
    HRESULT hr = CreateDeviceResources();

    if (FAILED(hr))
    {
        return hr;
    }

    g_pRenderTarget->BeginDraw();

    // 1. 清空背景
    g_pRenderTarget->Clear(
        D2D1::ColorF(0.10f, 0.13f, 0.18f)
    );

    // 2. 绘制蓝色填充矩形
    g_pBrush->SetColor(
        D2D1::ColorF(D2D1::ColorF::CornflowerBlue)
    );

    D2D1_RECT_F rcBlue = D2D1::RectF(
        50.0f, 50.0f,
        250.0f, 150.0f
    );

    g_pRenderTarget->FillRectangle(
        rcBlue,
        g_pBrush
    );

    // 3. 绘制绿色矩形边框
    g_pBrush->SetColor(
        D2D1::ColorF(D2D1::ColorF::LimeGreen)
    );

    D2D1_RECT_F rcGreen = D2D1::RectF(
        300.0f, 50.0f,
        500.0f, 150.0f
    );

    g_pRenderTarget->DrawRectangle(
        rcGreen,
        g_pBrush,
        3.0f
    );

    // 4. 绘制橙色圆形
    g_pBrush->SetColor(
        D2D1::ColorF(D2D1::ColorF::Orange)
    );

    D2D1_ELLIPSE ellipse = D2D1::Ellipse(
        D2D1::Point2F(150.0f, 250.0f),
        60.0f,
        60.0f
    );

    g_pRenderTarget->FillEllipse(
        ellipse,
        g_pBrush
    );

    // 5. 绘制白色直线
    g_pBrush->SetColor(
        D2D1::ColorF(D2D1::ColorF::White)
    );

    g_pRenderTarget->DrawLine(
        D2D1::Point2F(300.0f, 300.0f),
        D2D1::Point2F(500.0f, 200.0f),
        g_pBrush,
        3.0f
    );

    hr = g_pRenderTarget->EndDraw();

    // 渲染目标失效时释放设备相关资源
    if (hr == D2DERR_RECREATE_TARGET)
    {
        DiscardDeviceResources();
        hr = S_OK;
    }

    return hr;
}

// ============================================================
// 处理窗口尺寸变化
// ============================================================

void OnResize(UINT uWidth, UINT uHeight)
{
    if (g_pRenderTarget != nullptr)
    {
        HRESULT hr = g_pRenderTarget->Resize(
            D2D1::SizeU(uWidth, uHeight)
        );

        if (FAILED(hr))
        {
            DiscardDeviceResources();
        }
    }
}

// ============================================================
// 窗口过程函数
// ============================================================

LRESULT CALLBACK WndProc(
    HWND hWnd,
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam
)
{
    switch (uMsg)
    {
        case WM_SIZE:
        {
            UINT uWidth = LOWORD(lParam);
            UINT uHeight = HIWORD(lParam);

            OnResize(uWidth, uHeight);
            return 0;
        }

        case WM_PAINT:
        {
            PAINTSTRUCT psPaint = {};
            BeginPaint(hWnd, &psPaint);

            OnRender();

            EndPaint(hWnd, &psPaint);
            return 0;
        }

        case WM_DISPLAYCHANGE:
        {
            InvalidateRect(hWnd, nullptr, FALSE);
            return 0;
        }

        case WM_DESTROY:
        {
            DiscardDeviceResources();
            PostQuitMessage(0);
            return 0;
        }
    }

    return DefWindowProcW(
        hWnd,
        uMsg,
        wParam,
        lParam
    );
}

// ============================================================
// 程序入口
// ============================================================

int WINAPI wWinMain(
    HINSTANCE hInstance,
    HINSTANCE hPrevInstance,
    PWSTR pszCmdLine,
    int nCmdShow
)
{
    g_hInstance = hInstance;

    HRESULT hr = CreateDeviceIndependentResources();

    if (FAILED(hr))
    {
        MessageBoxW(
            nullptr,
            L"Direct2D Factory 创建失败！",
            L"错误",
            MB_OK | MB_ICONERROR
        );

        return -1;
    }

    const wchar_t* pszClassName = L"Direct2DWindow";

    WNDCLASSEXW wcWindow = {};
    wcWindow.cbSize = sizeof(WNDCLASSEXW);
    wcWindow.style = CS_HREDRAW | CS_VREDRAW;
    wcWindow.lpfnWndProc = WndProc;
    wcWindow.hInstance = hInstance;
    wcWindow.hCursor = LoadCursorW(nullptr, IDC_ARROW);
    wcWindow.lpszClassName = pszClassName;

    if (!RegisterClassExW(&wcWindow))
    {
        DiscardDeviceIndependentResources();
        return -1;
    }

    g_hWnd = CreateWindowExW(
        0,
        pszClassName,
        L"Direct2D 基础绘制",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        800,
        600,
        nullptr,
        nullptr,
        hInstance,
        nullptr
    );

    if (g_hWnd == nullptr)
    {
        DiscardDeviceIndependentResources();
        return -1;
    }

    ShowWindow(g_hWnd, nCmdShow);
    UpdateWindow(g_hWnd);

    MSG msg = {};

    while (GetMessageW(&msg, nullptr, 0, 0) > 0)
    {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }

    DiscardDeviceResources();
    DiscardDeviceIndependentResources();

    return static_cast<int>(msg.wParam);
}
