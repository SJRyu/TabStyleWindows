#pragma once

using IUnknown = ::IUnknown;

// Undefine GetCurrentTime macro to prevent
// conflict with Storyboard::GetCurrentTime
#undef GetCurrentTime

#include <winrt/Windows.System.h>
#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.Foundation.Collections.h>
#include <winrt/Windows.Foundation.Numerics.h>
#include <winrt/Windows.ApplicationModel.Activation.h>

#include <DispatcherQueue.h>

#include <winrt/base.h>
#include <winrt/Windows.UI.Composition.h>
#include <Windows.UI.Composition.Interop.h>
#include <winrt/Windows.UI.Composition.Desktop.h>
#include <winrt/Windows.UI.Composition.Interactions.h>
#include <winrt/Windows.UI.Input.h>
#include <winrt/Windows.Graphics.h>
#include <winrt/Windows.Graphics.Display.h>
#include <winrt/Windows.Graphics.DirectX.h>

