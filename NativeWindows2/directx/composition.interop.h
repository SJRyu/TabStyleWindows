#pragma once

#include <winrt/Windows.UI.Composition.h>
#include <winrt/Windows.UI.Composition.Desktop.h>
#include <winrt/Windows.Graphics.DirectX.h>
#include <windows.ui.composition.interop.h>

using namespace winrt;
using namespace Windows::Graphics;
using namespace Windows::Graphics::DirectX;
using namespace Windows::UI;
using namespace Windows::UI::Composition;
using namespace Windows::UI::Composition::Desktop;

namespace abidtop = ABI::Windows::UI::Composition::Desktop;
namespace abicomp = ABI::Windows::UI::Composition;

static __forceinline auto CreateDesktopWindowTarget(
    Compositor const& compositor, HWND hwnd, bool isTopmost = false)
{
    auto interop = compositor.as<abidtop::ICompositorDesktopInterop>();

    DesktopWindowTarget target{ nullptr };
    check_hresult(interop->CreateDesktopWindowTarget(
        hwnd, isTopmost, reinterpret_cast<abidtop::IDesktopWindowTarget**>(put_abi(target))));

    return target;
}

static __forceinline auto CreateCompositionGraphicsDevice(Compositor const& compositor, ::IUnknown* device)
{
    CompositionGraphicsDevice graphicsDevice{ nullptr };

    auto compositorInterop = compositor.as<abicomp::ICompositorInterop>();

    com_ptr<abicomp::ICompositionGraphicsDevice> graphicsInterop;

    check_hresult(compositorInterop->CreateGraphicsDevice(device, graphicsInterop.put()));

    check_hresult(graphicsInterop->QueryInterface(guid_of<CompositionGraphicsDevice>(),
        reinterpret_cast<void**>(put_abi(graphicsDevice))));

    return graphicsDevice;
}

static __forceinline auto CreateCompositionGraphicsDevice(
    abicomp::ICompositorInterop* compositorInterop, ::IUnknown* device)
{
    CompositionGraphicsDevice graphicsDevice{ nullptr };

    com_ptr<abicomp::ICompositionGraphicsDevice> graphicsInterop;

    check_hresult(compositorInterop->CreateGraphicsDevice(device, graphicsInterop.put()));

    check_hresult(graphicsInterop->QueryInterface(guid_of<CompositionGraphicsDevice>(),
        reinterpret_cast<void**>(put_abi(graphicsDevice))));

    return graphicsDevice;
}

static __forceinline auto CreateDrawingSurface(
    const CompositionGraphicsDevice& graphics,
    const Windows::Foundation::Size& size,
    const DirectXPixelFormat pixelfmt = DirectXPixelFormat::B8G8R8A8UIntNormalized,
    const DirectXAlphaMode alpha = DirectXAlphaMode::Premultiplied)
{
    auto surface = graphics.CreateDrawingSurface(size, pixelfmt, alpha);

    return surface;
}

static __forceinline auto CreateVirtualDrawingSurface(
    const CompositionGraphicsDevice& graphics,
    const SizeInt32& size,
    const DirectXPixelFormat pixelfmt = DirectXPixelFormat::B8G8R8A8UIntNormalized,
    const DirectXAlphaMode alpha = DirectXAlphaMode::Premultiplied)
{
    CompositionVirtualDrawingSurface surface = graphics.CreateVirtualDrawingSurface(size, pixelfmt, alpha);

    return surface;
}

static __forceinline auto CreateCompositionSurfaceForSwapChain(Compositor const& compositor, ::IUnknown* swapChain)
{
    ICompositionSurface surface{ nullptr };

    auto compositorInterop = compositor.as<abicomp::ICompositorInterop>();

    com_ptr<abicomp::ICompositionSurface> surfaceInterop;

    check_hresult(compositorInterop->CreateCompositionSurfaceForSwapChain(swapChain, surfaceInterop.put()));

    check_hresult(surfaceInterop->QueryInterface(
        guid_of<ICompositionSurface>(),
        reinterpret_cast<void**>(put_abi(surface))));

    return surface;
}

static __forceinline auto CreateCompositionSurfaceForSwapChain(
    abicomp::ICompositorInterop* compositorInterop, ::IUnknown* swapChain)
{
    ICompositionSurface surface{ nullptr };

    com_ptr<abicomp::ICompositionSurface> surfaceInterop;

    check_hresult(compositorInterop->CreateCompositionSurfaceForSwapChain(swapChain, surfaceInterop.put()));

    check_hresult(surfaceInterop->QueryInterface(
        guid_of<ICompositionSurface>(),
        reinterpret_cast<void**>(put_abi(surface))));

    return surface;
}

static auto CreateDispatcherQueueController()
{
    namespace abi = ABI::Windows::System;

    DispatcherQueueOptions options
    {
        sizeof(DispatcherQueueOptions),
        //DQTYPE_THREAD_DEDICATED,
        DQTYPE_THREAD_CURRENT,
        //DQTAT_COM_STA
        //DQTAT_COM_ASTA
        DQTAT_COM_NONE
    };

    Windows::System::DispatcherQueueController controller{ nullptr };
    //controller = Windows::System::DispatcherQueueController::CreateOnDedicatedThread();
    check_hresult(CreateDispatcherQueueController(options, reinterpret_cast<abi::IDispatcherQueueController**>(put_abi(controller))));
    return controller;
}

static auto CreateDispatcherQueueControllerOnDedicatedThread()
{
    namespace abi = ABI::Windows::System;

    DispatcherQueueOptions options
    {
        sizeof(DispatcherQueueOptions),
        DQTYPE_THREAD_DEDICATED,
        //DQTYPE_THREAD_CURRENT,
        //DQTAT_COM_STA
        DQTAT_COM_ASTA
        //DQTAT_COM_NONE
    };

    Windows::System::DispatcherQueueController controller{ nullptr };
    check_hresult(CreateDispatcherQueueController(options, reinterpret_cast<abi::IDispatcherQueueController**>(put_abi(controller))));
    return controller;
}
