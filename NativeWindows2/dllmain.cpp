// dllmain.cpp : DLL 애플리케이션의 진입점을 정의합니다.
#include <pch.h>
#include <NativeWindows2/windows/Win32Window.h>

HINSTANCE NativeWindows::g_hinst = nullptr;

static inline void UnregisterWindowsTypes()
{
    Win32Window::UnregisterWindowClass();
}

static inline void RegisterWindowsTypes()
{
    Win32Window::RegisterWindowClass();
}

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        g_hinst = hModule;
        RegisterWindowsTypes();
        break;
    case DLL_PROCESS_DETACH:
        UnregisterWindowsTypes();
        break;
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
        break;
    }
    return TRUE;
}

