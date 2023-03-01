#pragma once

#include <NativeWindows2/dllexport.h>

#define WIN32_LEAN_AND_MEAN             // 거의 사용되지 않는 내용을 Windows 헤더에서 제외합니다.
#define NOMINMAX
#include <sdkddkver.h>
#include <windows.h>
#include <windowsx.h>
#include <unknwn.h>
#include <restrictederrorinfo.h>
#include <hstring.h>

#include <stdlib.h>
#include <string>
#include <assert.h>
#include <utility>
#include <vector>
#include <stack>
#include <functional>

#include <objidl.h>
#include <Shlwapi.h>

#include <dwmapi.h>
#include <ShellScalingAPI.h>

//ppltask
#include <ppl.h>
#include <ppltasks.h>
#include <concrt.h>

#include <Uxtheme.h>
#include <vsstyle.h>

#include <wil/cppwinrt_helpers.h>

#include <NativeWindows2/template/templates.h>
using namespace Ctemplates;

#include <NativeWindows2/directx/DXimports.h>
#include <NativeWindows2/WinUIimports.h>
#include <NativeWindows2/directx/direct3d11.interop.h>
#include <NativeWindows2/directx/composition.interop.h>

namespace NativeWindows
{
	extern HINSTANCE NATIVEWINDOWS2_API g_hinst;
}
