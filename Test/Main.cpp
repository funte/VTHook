// 关闭重定义警告.
#pragma warning(disable : 4005)
#include <iostream>
#include <string>
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <debugapi.h>
#include <dxgi.h>
#include <dxgi1_3.h>
#include <dxgi1_4.h>
#include <dxgi1_6.h>
#pragma comment(lib, "DXGI.lib")
#include <d3d11.h>
#pragma comment (lib, "D3D11.lib")
#include <d3d12.h>
#pragma comment (lib, "D3D12.lib")
#include "hook.h"
#include "../common/utf8.hpp"
#include "../common/wrapper.hpp"

void listGPU()
{
	HRESULT hres;

	IDXGIFactory* pFactory = NULL;
	IDXGIAdapter* pAdapter = NULL;
	hres = CreateDXGIFactory(IID_PPV_ARGS(&pFactory));
	for (UINT id = 0; ; id++) {
		// Enumrate DXGI display adapter.
		hres = pFactory->EnumAdapters(id, &pAdapter); // offset 38h.
		if (S_OK == hres) {
			DXGI_ADAPTER_DESC adapterDesc;
			pAdapter->GetDesc(&adapterDesc); // offset 40h.
			//if (std::wstring(L"Microsoft Basic Render Driver") == adapterDesc.Description)
			//	continue;

			// Find the display adapter source name.
			DISPLAYCONFIG_SOURCE_DEVICE_NAME sourceName = {};
			sourceName.header.adapterId = adapterDesc.AdapterLuid;
			sourceName.header.type = DISPLAYCONFIG_DEVICE_INFO_GET_SOURCE_NAME;
			sourceName.header.size = sizeof(DISPLAYCONFIG_SOURCE_DEVICE_NAME);
			hres = DisplayConfigGetDeviceInfo(&sourceName.header);

			std::wcout
				<< L"  "
				<< sourceName.viewGdiDeviceName
				<< ": "
				<< adapterDesc.Description
				<< std::endl;
		} else {
			if (hres != DXGI_ERROR_NOT_FOUND) {
				std::cout << "  Failed to get next DXGI adapter, "
					<< "Err code = 0x"
					<< std::hex << hres << std::endl;
			}
			break;
		}
	}
}

// 用于替换 IsDebuggerPresent.
BEGIN_DEF_API_HOOK(BOOL WINAPI, IsDebuggerPresent)()
{
	return TRUE;
}
END_DEF_API_HOOK(kernel32, IsDebuggerPresent);

// 用于替换 IDXGIAdapter::GetDesc, VTable 索引为 8.
BEGIN_DEF_VT_HOOK(IDXGIAdapter, HRESULT WINAPI, GetDesc, 8)(
	IDXGIAdapter* pAdapter,
	_Out_ DXGI_ADAPTER_DESC* pDesc)
{
	HRESULT hres = CALL_VT_ORIGINAL(IDXGIAdapter, GetDesc, pAdapter)(pDesc);

	// 修改显卡名称.
	if (SUCCEEDED(hres) && pDesc) {
		HRESULT hres2 = ERROR_SUCCESS;
		std::swprintf(pDesc->Description, 128, L"NVIDIA GeForce RTX 4090TI");
	}

	return hres;
};
END_DEF_VT_HOOK(IDXGIAdapter, GetDesc);

// 用于替换 IDXGIFactory::EnumAdapters, VTable 索引为 7.
BEGIN_DEF_VT_HOOK(IDXGIFactory, HRESULT WINAPI, EnumAdapters, 7)(
	IDXGIFactory* pclsObj,
	UINT Adapter,
	_COM_Outptr_ IDXGIAdapter** ppAdapter)
{
	HRESULT hres = CALL_VT_ORIGINAL(IDXGIFactory, EnumAdapters, pclsObj)(
		Adapter, ppAdapter);
	if (SUCCEEDED(hres)) {
		// 替换 IDXGIAdapter::GetDesc.
		HOOK_VT_TARGET(IDXGIAdapter, GetDesc, *ppAdapter);
	}
	return hres;
}
END_DEF_VT_HOOK(IDXGIFactory, EnumAdapters);

// 用于替换 CreateDXGIFactory 函数.
BEGIN_DEF_API_HOOK(HRESULT WINAPI, CreateDXGIFactory)(
	// __uuidof(IDXGIFactory)="7b7166ec-21c7-44ae-b21a-c9ae321ae369".
	REFIID riid,
	_Out_ void** ppFactory)
{
	HRESULT hres = API_ORIGINAL(CreateDXGIFactory)(riid, ppFactory);
	if (SUCCEEDED(hres)) {
		// 替换 IDXGIFactory::EnumAdapters.
		HOOK_VT_TARGET(IDXGIFactory, EnumAdapters, *ppFactory);
	}
	return hres;
}
END_DEF_API_HOOK(dxgi, CreateDXGIFactory);

int main()
{
	// 初始化 MinHook.
	auto status = MH_Initialize();
	if (MH_OK != status) {
		dlogp("MH_Initialize failed, status: %s", MH_StatusToString(status));
		return FALSE;
	}

	// 输出: Before hook, IsDebuggerPresent return 0
	std::cout << "Before hook, IsDebuggerPresent return " << IsDebuggerPresent() << std::endl;
	// 执行 hook.
	HOOK_API(IsDebuggerPresent);
	// 输出: After hook, IsDebuggerPresent return 1
	std::cout << "After hook, IsDebuggerPresent return " << IsDebuggerPresent() << std::endl;

	std::cout << "Before hook, list GPU:" << std::endl;
	// 输出:
	//\\.\DISPLAY1: NVIDIA GeForce GTX 1660 Ti
	//\\.\DISPLAY5: NVIDIA GeForce GTX 1660 Ti
	//: Microsoft Basic Render Driver
	listGPU();
	// 执行 hook.
	HOOK_API(CreateDXGIFactory);
	std::cout << "After hook, list GPU:" << std::endl;
	// 输出:
	//\\.\DISPLAY1: NVIDIA GeForce RTX 4090TI
	//\\.\DISPLAY5: NVIDIA GeForce RTX 4090TI
	//: NVIDIA GeForce RTX 4090TI
	listGPU();
}