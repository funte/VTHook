#pragma once
#include "./dlog.hpp"
#include "./hook.h"

#define API_TARGET(Function) target_ ## Function
#define API_ORIGINAL(Function) original_ ## Function
#define API_DETOUR(Function) detour_ ## Function
#define API_HOOKER(Function) hook_ ## Function
#define API_ENABLER(Function) enable_ ## Function
#define API_REMOVER(Function) remove_ ## Function
// 注意: 这里必须提前引用 Function, 否则替换 RegGetValueW 等函数可能会显示
// MH_ERROR_MODULE_NOT_FOUND 错误, 这可能与编译器的优化有关.
#define BEGIN_DEF_API_HOOK(ReturnType, Function) \
	PVOID API_TARGET(Function) = NULL; \
	decltype(&Function) API_ORIGINAL(Function) = NULL; \
	ReturnType API_DETOUR(Function)
#define END_DEF_API_HOOK(DLL, Function) \
	MH_STATUS API_HOOKER(Function)() { \
		auto status = MH_CreateHookApiEx( \
			L ### DLL, \
			# Function, \
			API_DETOUR(Function), \
			(LPVOID*)&API_ORIGINAL(Function), \
			(LPVOID*)&API_TARGET(Function)); \
		if (MH_OK == status) { \
			dlogpw(L"Create API hook %s:%s", L ### DLL, L ### Function); \
		} else { \
			dlogpw(L"Failed to create API hook %s:%s, status: %s", L ### DLL, L ### Function, MH_StatusToWString(status)); \
		} \
		return status; \
	} \
	MH_STATUS API_ENABLER(Function)() { \
		auto status = MH_EnableHook(API_TARGET(Function)); \
		if (MH_OK == status) { \
			dlogpw(L"Hook API %s:%s", L ### DLL, L ### Function); \
		} else { \
			dlogpw(L"Failed to hook API %s:%s, status: %s", L ### DLL, L ### Function, MH_StatusToWString(status)); \
		} \
		return status; \
	} \
	MH_STATUS API_REMOVER(Function)() { \
		return MH_RemoveHook(API_TARGET(Function)); \
	}
#define HOOK_API(Function) \
	API_REMOVER(Function)(); \
	API_HOOKER(Function)(); \
	API_ENABLER(Function)();

#define VT_MethodOffset_To_MethodIdx(methodOffset) ((int)(methodOffset/sizeof(void*)))
#define VT_MethodIdx(CLS, Method) methodIdx_ ## CLS ## _ ## Method
#define VT_TARGET(CLS, Method) target_ ## CLS ## _ ## Method
#define VT_ORIGINAL(CLS, Method) original_ ## CLS ## _ ## Method
#define VT_DETOUR(CLS, Method) detour_ ## CLS ## _ ## Method
#define CALL_VT_ORIGINAL(CLS, Method, pclsObj) (pclsObj->*VT_ORIGINAL(CLS, Method))
#define VT_TARGET_HOOKER(CLS, Method) hook_ ## CLS ## _ ## Method
#define VT_TARGET_ENABLER(CLS, Method) enable_ ## CLS ## _ ## Method
#define VT_TARGET_REMOVER(CLS, Method) remove_ ## CLS ## _ ## Method
#define BEGIN_DEF_VT_HOOK(CLS, ReturnType, Method, MethodIdx) \
	int VT_MethodIdx(CLS, Method) = MethodIdx; \
	LPVOID VT_TARGET(CLS, Method) = NULL; \
	decltype(& ## CLS ## :: ## Method) VT_ORIGINAL(CLS, Method) = NULL; \
	ReturnType VT_DETOUR(CLS, Method)
#define END_DEF_VT_HOOK(CLS, Method) \
	MH_STATUS VT_TARGET_HOOKER(CLS, Method)() { \
		auto status = MH_CreateHook(VT_TARGET(CLS, Method), VT_DETOUR(CLS, Method), (LPVOID*)&VT_ORIGINAL(CLS, Method)); \
		if (MH_OK == status) { \
			dlogpw(L"Create VTable hook %s::%s", L ### CLS, L ### Method); \
		} else { \
			dlogpw(L"Failed to create VTable hook %s:%s, status: %s", L ### CLS, L ### Method, MH_StatusToWString(status)); \
		} \
		return status; \
	} \
	MH_STATUS VT_TARGET_ENABLER(CLS, Method)() { \
		auto status = MH_EnableHook(VT_TARGET(CLS, Method)); \
		if (MH_OK == status) { \
			dlogpw(L"Hook VTable %s::%s", L ### CLS, L ### Method); \
		} else { \
			dlogpw(L"Failed to hook VTable %s:%s, status: %s", L ### CLS, L ### Method, MH_StatusToWString(status)); \
		} \
		return status; \
	} \
	MH_STATUS VT_TARGET_REMOVER(CLS, Method)() { \
		return MH_RemoveHook(VT_TARGET(CLS, Method)); \
	}
// 注意: 每次调用 CoUninitialize 关闭 COM 库时, 所有相关资源都会被释放, 包括被
// 使用 MinHook 库安装的那些 VT 钩子, 为了与 MinHook 缓存中的数据同步,
// 这些钩子也必须提前清除.
#define HOOK_VT_TARGET(CLS, Method, pclsObj) \
	VT_TARGET(CLS, Method) = (((LPVOID*)((((LPVOID*)pclsObj)[0])))[VT_MethodIdx(CLS, Method)]); \
	VT_TARGET_REMOVER(CLS, Method)(); \
	VT_TARGET_HOOKER(CLS, Method)(); \
	VT_TARGET_ENABLER(CLS, Method)();
