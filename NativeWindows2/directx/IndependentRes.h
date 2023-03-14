#pragma once

#include <NativeWindows2/WinUIimports.h>
#include <NativeWindows2/directx/DXimports.h>
#include <NativeWindows2/directx/composition.interop.h>

namespace NativeWindows
{
	using namespace winrt;
	using namespace Windows::System;
	using namespace Windows::UI::Composition;
	namespace abicomp = ABI::Windows::UI::Composition;

	class NATIVEWINDOWS2_API IndependentRes
	{
	public:

		inline IndependentRes() {};
		inline virtual ~IndependentRes()
		{
		};

		IndependentRes(const IndependentRes& r) = delete;
		IndependentRes& operator=(const IndependentRes& r) = delete;

		inline void InitIndependentRes(bool bMultithreaded = false)
		{
			Throwif(d3dDevice_.get());
			int d3dflags = D3D11_CREATE_DEVICE_BGRA_SUPPORT | D3D11_CREATE_DEVICE_VIDEO_SUPPORT;

			if (bMultithreaded)
			{
				CreateD2dFactory(D2D1_FACTORY_TYPE_MULTI_THREADED);
			}
			else
			{
				CreateD2dFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED);
				d3dflags |= D3D11_CREATE_DEVICE_SINGLETHREADED;
			}

			CreateDwriteFactory(DWRITE_FACTORY_TYPE_SHARED);
			CreateDevice(
				D3D11_CREATE_DEVICE_SINGLETHREADED |
				D3D11_CREATE_DEVICE_BGRA_SUPPORT |
				D3D11_CREATE_DEVICE_VIDEO_SUPPORT);
			HR(dxgiDevice_->SetMaximumFrameLatency(1));

			CreateCompositor();
		}

		inline void InitIndependetResMinimum()
		{
			CreateDevice(
				D3D11_CREATE_DEVICE_SINGLETHREADED |
				D3D11_CREATE_DEVICE_BGRA_SUPPORT |
				D3D11_CREATE_DEVICE_VIDEO_SUPPORT);
		}

		inline void ReleaseIndependentRes()
		{
#ifdef WINRT_CLOSETEST
			ReleaseCompositor();
			//ReleaseFactory();
			//ReleaseDevice();
#endif
		}

		inline auto TestSwapchainForHwnd(HWND hwnd)
		{
			DXGI_SWAP_CHAIN_DESC1 desc{ 0, };
			desc.Width = 4;
			desc.Height = 4;
			desc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
			desc.SampleDesc.Count = 1;
			desc.SampleDesc.Quality = 0;
			desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
			desc.BufferCount = 3;
			desc.Scaling = DXGI_SCALING_NONE;
			desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
			desc.AlphaMode = DXGI_ALPHA_MODE_IGNORE;
			desc.Flags = DXGI_SWAP_CHAIN_FLAG_DISPLAY_ONLY;

			IDXGISwapChain1* sc;
			HR(dxgiFactory_->CreateSwapChainForHwnd(
				d3dDevice_.get(),
				hwnd,
				&desc,
				nullptr,
				nullptr,
				&sc));
			/**
			* RSJ: Alt + Enter 이후에 상태가 안좋아진다. 리사이징이 버벅인다.
			**/
			dxgiFactory_->MakeWindowAssociation(hwnd,
				//DXGI_MWA_NO_WINDOW_CHANGES);
				DXGI_MWA_NO_ALT_ENTER | DXGI_MWA_NO_PRINT_SCREEN);

			return sc;
		}

		com_ptr<IDXGIAdapter4> adapter_;
		com_ptr<IDXGIFactory7> dxgiFactory_;
		//com_ptr<IDXGIFactoryMedia> dxgiMediaFactory_;

		com_ptr<ID2D1Factory7> d2dFactory_;
		com_ptr<ID2D1Device6> d2dDevice_;
		com_ptr<IDWriteFactory7> dwriteFactory_;

		com_ptr<ID3D11Device5> d3dDevice_;
		com_ptr<ID3D11DeviceContext4> d3dDc_;
		com_ptr<IDXGIDevice4> dxgiDevice_;

		Compositor compositor_{ nullptr };
		com_ptr<abicomp::ICompositorInterop> compositori_;
		CompositionGraphicsDevice graphics_{ nullptr };

	protected:


	private:

		inline void CreateD2dFactory(D2D1_FACTORY_TYPE type)
		{
			D2D1_FACTORY_OPTIONS options;
#ifdef _DEBUG
			options.debugLevel = D2D1_DEBUG_LEVEL_INFORMATION;
#else
			options.debugLevel = D2D1_DEBUG_LEVEL_NONE;
#endif
			HR(D2D1CreateFactory(
				type,
				__uuidof(d2dFactory_.get()),
				&options,
				d2dFactory_.put_void()));
		}

		inline void CreateDwriteFactory(DWRITE_FACTORY_TYPE type)
		{
			HR(DWriteCreateFactory(
				type,
				__uuidof(dwriteFactory_.get()),
				(::IUnknown**)(dwriteFactory_.put())));
		}

		inline void ReleaseFactory()
		{
			dwriteFactory_ = nullptr;
			d2dFactory_ = nullptr;
		}

		inline void CreateDevice(int flags)
		{
#ifdef _DEBUG
			flags |= D3D11_CREATE_DEVICE_DEBUG;
#endif
			D3D_FEATURE_LEVEL featureLevel =
				D3D_FEATURE_LEVEL_11_1;

			ID3D11Device* device;
			ID3D11DeviceContext* ctx;
			HR(D3D11CreateDevice(
				nullptr,    // Adapter
				D3D_DRIVER_TYPE_HARDWARE,
				nullptr,    // Module
				flags,
				&featureLevel, 1, // Highest available feature level, 
				D3D11_SDK_VERSION,
				&device,
				nullptr,    // Actual feature level
				&ctx		// device contex
			));
			if (!(flags & D3D11_CREATE_DEVICE_SINGLETHREADED))
			{
				ID3D10Multithread* multi;
				device->QueryInterface(&multi);
				multi->SetMultithreadProtected(true);
				multi->Release();
			}

			HR(ctx->QueryInterface(d3dDc_.put()));
			ctx->Release();

			HR(device->QueryInterface(d3dDevice_.put()));
			device->Release();

			// dxgi device
			HR(d3dDevice_->QueryInterface(dxgiDevice_.put()));


			// get dxgiadapter 
			IDXGIAdapter* adapter;
			HR(dxgiDevice_->GetAdapter(&adapter));
			HR(adapter->QueryInterface(adapter_.put()));
			adapter->Release();

			// get dxgifactory
			HR(adapter_->GetParent(__uuidof(dxgiFactory_.get()), dxgiFactory_.put_void()));
			//HR(dxgiFactory_->QueryInterface(dxgiMediaFactory_.put()));

			// d2d device
			if (d2dFactory_.get() != nullptr)
				HR(d2dFactory_->CreateDevice(dxgiDevice_.get(), d2dDevice_.put()));
		}

		inline void ReleaseDevice()
		{
			d2dDevice_ = nullptr;
			adapter_ = nullptr;
			//dxgiMediaFactory_ = nullptr;
			dxgiFactory_ = nullptr;
			dxgiDevice_ = nullptr;
			d3dDc_ = nullptr;
			d3dDevice_ = nullptr;
		}

		inline void CreateCompositor()
		{
			compositor_ = Compositor();
			compositori_ = compositor_.as<abicomp::ICompositorInterop>();

			graphics_ = CreateCompositionGraphicsDevice(compositori_.get(), d2dDevice_.get());
		}

		inline void ReleaseCompositor()
		{
			graphics_.Close();
			compositori_ = nullptr;
			compositor_.Close();
		}

	};
}
