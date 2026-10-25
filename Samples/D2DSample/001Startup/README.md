### 1. ID2D1Factory是什么?
在 Direct2D（D2D）中，ID2D1Factory 是一个工厂接口（Factory Interface），主要负责创建 Direct2D 绘图所需要的各种资源对象。
可以理解为：整个 D2D 绘图系统的入口，负责创建绘图相关的对象，但它本身不负责绘制图形

ID2D1Factory的初始化如下:

```cpp
ID2D1Factory *pFactory = nullptr;
D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &pFactory);
if (!pFactory)
	throw std::runtime_error("D1D1CreateFactory Failed");
```


初始化成功后, 可以创建绘图相关对象了

- pFactory-->CreateHwndRenderTarget()  创建 ID2D1HwndRenderTarget 对象, 向窗口绘图
- pFactory-->CreateDCRenderTarget()   创建 ID2D1DcRenderTarget对象, 向GDI设备上下文绘图
- 

### 2. ID2D1HwndRenderTarget是什么?

ID2D1HwndRenderTarget 是 Direct2D 中专门用于向 Win32 窗口（HWND）绘图的渲染目标接口。
它继承自 ID2D1RenderTarget，因此不仅能够向窗口绘制图形，还能够创建笔刷、位图等绘图资源。
可以把它理解为窗口的绘图引擎：你告诉它绘制什么、使用什么颜色、画在哪里，它就负责把内容渲染到窗口上。