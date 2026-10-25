#pragma once
#include <objbase.h>





interface IFoo : IUnknown
{
	virtual HRESULT __stdcall SayHello() = 0;
	virtual HRESULT __stdcall Add(int a, int b, int* result) = 0;
};

//接口ID声明
// {90CEE3F1-F3C0-47EF-B9B7-262E0A458908}
static const GUID IID_IFoo =
{ 0x90cee3f1, 0xf3c0, 0x47ef, { 0xb9, 0xb7, 0x26, 0x2e, 0xa, 0x45, 0x89, 0x8 } };


//类ID声明
// {3D384E52-CC4D-4311-8343-4624FE990C80}
static const GUID  CLSID_Foo  =
{ 0x3d384e52, 0xcc4d, 0x4311, { 0x83, 0x43, 0x46, 0x24, 0xfe, 0x99, 0xc, 0x80 } };
