>[中文](README.md)|English

# VTHook
Base on `MinHook`, add some marcos to support hook C++ Virtual Tables and COM class instance.

## Build
1. Requirements: `cmkr`, `cmake`, `vs2022`.  
2. Hook windows api, such as `IsDebuggerPresent`, make it always return `TRUE`:
  * Define hook code:
  ```cpp
  // To replace IsDebuggerPresent.
  BEGIN_DEF_API_HOOK(BOOL WINAPI, IsDebuggerPresent)()
  {
    return TRUE;
  }
  END_DEF_API_HOOK(kernel32, IsDebuggerPresent);

  ```
  * Perform hook:
  ```cpp
  // Output: Before hook, IsDebuggerPresent return 0
  std::cout << "Before hook, IsDebuggerPresent return " << IsDebuggerPresent() << std::endl;
  // Perform hook.
  HOOK_API(IsDebuggerPresent);
  // Output: After hook, IsDebuggerPresent return 1
  std::cout << "After hook, IsDebuggerPresent return " << IsDebuggerPresent() << std::endl;
  ```
3. Hook C++ Virtual Tables and COM class instance, such as `IDXGIAdapter::GetDesc`, rename the description to `NVIDIA GeForce RTX 4090TI`.  
  * Define hook code:  
  💡How dump virtual table in vs2022 compilation, see https://stackoverflow.com/a/302607/5906199.
  ```cpp
  // To relplace IDXGIAdapter::GetDesc, VTable index is 8.
  BEGIN_DEF_VT_HOOK(IDXGIAdapter, HRESULT WINAPI, GetDesc, 8)(
    IDXGIAdapter* pAdapter,
    _Out_ DXGI_ADAPTER_DESC* pDesc)
  {
    HRESULT hres = CALL_VT_ORIGINAL(IDXGIAdapter, GetDesc, pAdapter)(pDesc);

    // Rename description.
    if (SUCCEEDED(hres) && pDesc) {
      HRESULT hres2 = ERROR_SUCCESS;
      std::swprintf(pDesc->Description, 128, L"NVIDIA GeForce RTX 4090TI");
    }

    return hres;
  };
  END_DEF_VT_HOOK(IDXGIAdapter, GetDesc);

  // To replace IDXGIFactory::EnumAdapters, VTable index 7.
  BEGIN_DEF_VT_HOOK(IDXGIFactory, HRESULT WINAPI, EnumAdapters, 7)(
    IDXGIFactory* pclsObj,
    UINT Adapter,
    _COM_Outptr_ IDXGIAdapter** ppAdapter)
  {
    HRESULT hres = CALL_VT_ORIGINAL(IDXGIFactory, EnumAdapters, pclsObj)(
      Adapter, ppAdapter);
    if (SUCCEEDED(hres)) {
      // Perform hook IDXGIAdapter::GetDesc.
      HOOK_VT_TARGET(IDXGIAdapter, GetDesc, *ppAdapter);
    }
    return hres;
  }
  END_DEF_VT_HOOK(IDXGIFactory, EnumAdapters);

  // To replace CreateDXGIFactory.
  BEGIN_DEF_API_HOOK(HRESULT WINAPI, CreateDXGIFactory)(
    // __uuidof(IDXGIFactory)="7b7166ec-21c7-44ae-b21a-c9ae321ae369".
    REFIID riid,
    _Out_ void** ppFactory)
  {
    HRESULT hres = API_ORIGINAL(CreateDXGIFactory)(riid, ppFactory);
    if (SUCCEEDED(hres)) {
      // Perform hook IDXGIFactory::EnumAdapters.
      HOOK_VT_TARGET(IDXGIFactory, EnumAdapters, *ppFactory);
    }
    return hres;
  }
  END_DEF_API_HOOK(dxgi, CreateDXGIFactory);
  ```
  * Perform hook:
  ```cpp
	std::cout << "Before hook, list GPU:" << std::endl;
	// Output:
	//\\.\DISPLAY1: NVIDIA GeForce GTX 1660 Ti
	//\\.\DISPLAY5: NVIDIA GeForce GTX 1660 Ti
	//: Microsoft Basic Render Driver
	listGPU();
	// Perform hook.
	HOOK_API(CreateDXGIFactory);
	std::cout << "After hook, list GPU:" << std::endl;
	// Output:
	//\\.\DISPLAY1: NVIDIA GeForce RTX 4090TI
	//\\.\DISPLAY5: NVIDIA GeForce RTX 4090TI
	//: NVIDIA GeForce RTX 4090TI
	listGPU();
  ```
4. Compile project, change to root directory, execute the script `build.cmd` in command line;  
5. If all goes well, execute `"./build64/Release/Test.exe"` in command line and you will see the output:
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
