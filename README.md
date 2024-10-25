>中文|[English](README.en.md)

# VTHook
基于 `MinHook`, 添加一些宏代码, 支持 hook C++ 虚函数表和 COM 对象.

## 如何编译
1. 环境要求: `cmkr`, `cmake`, `vs2022`.  
2. hook 普通函数, 这里以 `IsDebuggerPresent` 为例, hook 之后让它总是返回 `TRUE`:
  * 定义 hook 代码:
  ```cpp
  // 用于替换 IsDebuggerPresent.
  BEGIN_DEF_API_HOOK(BOOL WINAPI, IsDebuggerPresent)()
  {
    return TRUE;
  }
  END_DEF_API_HOOK(kernel32, IsDebuggerPresent);

  ```
  * 执行 hook:
  ```cpp
  // 输出: Before hook, IsDebuggerPresent return 0
  std::cout << "Before hook, IsDebuggerPresent return " << IsDebuggerPresent() << std::endl;
  // 执行 hook.
  HOOK_API(IsDebuggerPresent);
  // 输出: After hook, IsDebuggerPresent return 1
  std::cout << "After hook, IsDebuggerPresent return " << IsDebuggerPresent() << std::endl;
  ```
3. hook 虚函数表和 COM 对象, 这里以 `IDXGIAdapter::GetDesc` 为例, 将显卡名称修改为 `NVIDIA GeForce RTX 4090TI`.  
  * 定义 hook 代码:  
  💡如何在 vs2022 编译时打印虚函数表, 参考 https://stackoverflow.com/a/302607/5906199.
  ```cpp
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
  ```
  * 执行 hook:
  ```cpp
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
  ```
4. 编译项目, 进入根目录, 在命令行中执行 `build.cmd` 脚本;  
5. 如果一切顺利, 在命令行中执行 `"./build64/Release/Test.exe"` 会看到下列输出:
```sh
Before hook, IsDebuggerPresent return 0
After hook, IsDebuggerPresent return 1
Before hook, list GPU:
  \\.\DISPLAY1: NVIDIA GeForce GTX 1660 Ti
  \\.\DISPLAY5: NVIDIA GeForce GTX 1660 Ti
  : Microsoft Basic Render Driver
After hook, list GPU:
  \\.\DISPLAY1: NVIDIA GeForce RTX 4090TI
  \\.\DISPLAY5: NVIDIA GeForce RTX 4090TI
  : NVIDIA GeForce RTX 4090TI
```
