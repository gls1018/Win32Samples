#include <windows.h>
#include <gdiplus.h>
#include <stdexcept>
#pragma comment(lib, "Gdiplus.lib")    // 需要链接 Gdiplus.lib

ULONG_PTR gdiplusToken = 0;

int main()
{

	// 1. 使用GDI+ 之前, 必须调用 GdiplusStartup
	Gdiplus::GdiplusStartupInput input;
	if (Gdiplus::GdiplusStartup(&gdiplusToken, &input, nullptr) != Gdiplus::Ok)
		throw std::runtime_error("GdiplusStartup Failed");

	// 2. 在这里执行 Win32 程序的主体逻辑， 例如: 注册窗口类、创建窗口、消息循环


	// 3. 消息循环退出后, 调用GdiplusShutdown
	Gdiplus::GdiplusShutdown(gdiplusToken);
	return 0;
}