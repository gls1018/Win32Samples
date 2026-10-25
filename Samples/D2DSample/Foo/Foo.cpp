#include "IFoo.h"
#include <windows.h>
#include <Unknwn.h>
#include <d2d1.h>
#include <iostream>

//引用计数
long g_RefCount = 0;


//实现IFoo接口的类
class CFoo :public IFoo
{
public:
	CFoo() :m_RefCount(1)
	{
		InterlockedIncrement(&g_RefCount);
	}
	~CFoo()
	{
		InterlockedDecrement(&g_RefCount);
	}


	//IUnknown 方法
	HRESULT __stdcall QueryInterface(const IID& iid, void** ppv) override
	{
		if (iid == IID_IUnknown || iid==IID_IFoo)
		{
			*ppv = static_cast<IFoo*>(this);
			AddRef();
			return S_OK;
		}
		*ppv = nullptr;
		return E_NOINTERFACE;
	}

	ULONG __stdcall AddRef() override
	{
		return InterlockedIncrement(&m_RefCount);
	}

	ULONG __stdcall Release() override
	{
		ULONG count = InterlockedDecrement(&m_RefCount);
		if (m_RefCount == 0)
			delete this;
		return count;
	}

	// IFoo 方法
	HRESULT __stdcall SayHello()override
	{
		std::cout << "Hello from COM component \n";
		return S_OK;
	}

	HRESULT __stdcall Add(int a, int b, int* result) override
	{
		if (!result) return E_POINTER;
		*result = a + b;
		return S_OK;
	}

private:
	long m_RefCount;
};



// 类工厂
class CFooFactory : public IClassFactory
{
public:
	CFooFactory() : m_RefCount(1) { }

	// IUnknown 方法
	HRESULT __stdcall QueryInterface(const IID& iid, void** ppv) override
	{
		if (iid == IID_IUnknown || iid == IID_IClassFactory)
		{
			*ppv = static_cast<IClassFactory*>(this);
			AddRef();
			return S_OK;
		}
		return E_NOINTERFACE;
	}

	ULONG __stdcall AddRef()
	{
		InterlockedIncrement(&m_RefCount);
	}

	ULONG __stdcall Release()
	{
		ULONG count = InterlockedDecrement(&m_RefCount);
		if (count == 0)
			delete this;
		return count;
	}


	//IClassFactory 方法
	HRESULT __stdcall CreateInstance(IUnknown* pUnkOuter, const IID& iid, void** ppv) override
	{
		if (pUnkOuter != nullptr)
			return CLASS_E_NOAGGREGATION;

		CFoo* pFoo = new CFoo();
		if (!pFoo)
			return E_OUTOFMEMORY;

		HRESULT hr = pFoo->QueryInterface(iid, ppv);
		pFoo->Release();

		return hr;
	}

private:
	long m_RefCount;
};