#include <windows.h>
#include "dll.h"
#include "factory.h"
#include "register.h"

// Required globals
long g_DllRef = 0;
CRITICAL_SECTION g_ShellCs;
CSXGLOBALS globals = {NULL, 0, false, NULL, false};


// Dll entry point.
BOOL APIENTRY DllMain(HMODULE module, DWORD reason, LPVOID reserved)
{
    switch (reason)
    {
    case DLL_PROCESS_ATTACH:
        globals.module = module;
        InitializeCriticalSection(&g_ShellCs);
        DisableThreadLibraryCalls(module);
        break;
    case DLL_THREAD_ATTACH:
        break;
    case DLL_THREAD_DETACH:
        break;
    case DLL_PROCESS_DETACH:
        DeleteCriticalSection(&g_ShellCs);
        if (globals.shield)
            DeleteObject(globals.shield);
        break;
    }
    return TRUE;
}


// Create the class factory and query to the specific interface.
STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, void **ppv)
{
    HRESULT hr = CLASS_E_CLASSNOTAVAILABLE;

    if (IsEqualCLSID(COMPOSER_CLSID, rclsid))
    {
        hr = E_OUTOFMEMORY;

        ClassFactory *pClassFactory = new ClassFactory();
        if (pClassFactory)
        {
            hr = pClassFactory->QueryInterface(riid, ppv);
            pClassFactory->Release();
        }
    }

    return hr;
}


// Check if we can unload the component from the memory. 
STDAPI DllCanUnloadNow(void)
{
    return g_DllRef > 0 ? S_FALSE : S_OK;
}


// Register the COM server and the context menu handler. 
STDAPI DllRegisterServer(void)
{
    HRESULT hr;

    wchar_t module[MAX_PATH];
    if (GetModuleFileName(globals.module, module, ARRAYSIZE(module)) == 0)
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
    }
    else
    {
        ComposerShellReg registry;
        hr = registry.Register(module);
    }
    
    return hr;
}


// Unregister the COM server and the context menu handler.
STDAPI DllUnregisterServer(void)
{
    ComposerShellReg registry;
    return registry.Unregister();
}
