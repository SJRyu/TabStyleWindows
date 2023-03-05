#include <pch.h>
#include <NativeWindows2/directx/VideoComponents.h>
#include <NativeWindows2/windows/Win32Window.h>
#include <NativeWindows2/Win32UIThread.h>

VideoComponents::VideoComponents(Win32Window* window) :
	window_(window)
{
}


VideoComponents::~VideoComponents()
{
}

void VideoComponents::CreateDxResources()
{
	InitDevice();
	InitCompositor();
	CreateSwapchain(1920, 1088);
}

void VideoComponents::InitDevice()
{
	UINT flags = D3D11_CREATE_DEVICE_VIDEO_SUPPORT;

#ifdef _DEBUG
	flags |= D3D11_CREATE_DEVICE_DEBUG;
#endif
	D3D_FEATURE_LEVEL featureLevel =
		D3D_FEATURE_LEVEL_11_1;

	ID3D11Device* device;
	ID3D10Multithread* multi;
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

	device->QueryInterface(&multi);
	multi->SetMultithreadProtected(true);
	multi->Release();

	HR(device->QueryInterface(d3dDevice_.put()));
	device->Release();

	HR(ctx->QueryInterface(d3dDc_.put()));
	ctx->Release();

	// dxgi device
	HR(d3dDevice_->QueryInterface(dxgiDevice_.put()));

	// get dxgiadapter 
	IDXGIAdapter* adapter;
	HR(dxgiDevice_->GetAdapter(&adapter));
	HR(adapter->QueryInterface(adapter_.put()));
	adapter->Release();

	// get dxgifactory
	HR(adapter_->GetParent(__uuidof(dxgiFactory_.get()), dxgiFactory_.put_void()));
	HR(dxgiFactory_->QueryInterface(dxgiMediaFactory_.put()));
}

void VideoComponents::InitCompositor()
{
	SafeClose(&hcomposition_);

	HR(DCompositionCreateSurfaceHandle(
		COMPOSITIONSURFACE_ALL_ACCESS, nullptr, &hcomposition_));

	compositor_ = Compositor();
	compositori_ = compositor_.as<abicomp::ICompositorInterop>();

	target_ = CreateDesktopWindowTarget(compositor_, window_->hwnd_);
	rootv_ = compositor_.CreateContainerVisual();
	rootv_.RelativeSizeAdjustment({ 1.0f, 1.0f });

	target_.Root(rootv_);
	visuals_ = rootv_.Children();

	videovisual_ = compositor_.CreateSpriteVisual();
	videovisual_.RelativeSizeAdjustment({ 1.0f, 1.0f });

	videobrush_ = compositor_.CreateSurfaceBrush();
	videobrush_.Stretch(CompositionStretch::Fill);

	videovisual_.Brush(videobrush_);
	visuals_.InsertAtTop(videovisual_);
}

void VideoComponents::CreateSwapchain(LONG width, LONG height, DXGI_FORMAT format)
{
	scdesc_.Width = width;
	scdesc_.Height = height;
	scdesc_.Format = format;
	scdesc_.SampleDesc.Count = 1;
	scdesc_.SampleDesc.Quality = 0;
	scdesc_.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	scdesc_.BufferCount = 2;
	scdesc_.Scaling = DXGI_SCALING_STRETCH;
	scdesc_.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	scdesc_.AlphaMode = DXGI_ALPHA_MODE_IGNORE;
	scdesc_.Flags = DXGI_SWAP_CHAIN_FLAG_YUV_VIDEO |
		DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING |
		DXGI_SWAP_CHAIN_FLAG_FRAME_LATENCY_WAITABLE_OBJECT;

	IDXGISwapChain1* sc;
	HR(dxgiMediaFactory_->CreateSwapChainForCompositionSurfaceHandle(
		d3dDevice_.get(),
		hcomposition_,
		&scdesc_,
		nullptr,
		&sc));
	HR(sc->QueryInterface(videosc_.put()));
	sc->Release();

	HR(videosc_->SetMaximumFrameLatency(1));
	HR(videosc_->GetBuffer(0, __uuidof(backbuffer_.get()), backbuffer_.put_void()));

	HR(videosc_->Present(0, 0));

	scsurface_ = CreateCompositionSurfaceForSwapChain(compositori_.get(), videosc_.get());
	videobrush_.Surface(scsurface_);
}

void VideoComponents::ReleaseDxResources()
{
	ReleaseSwapchain();
	ReleaseCompositor();
	//ReleaseDevice();
}

void VideoComponents::ReleaseSwapchain()
{
	videobrush_.Surface(nullptr);
	scsurface_ = nullptr;
	backbuffer_ = nullptr;
	videosc_ = nullptr;
}

void VideoComponents::ReleaseCompositor()
{
	videovisual_.Brush(nullptr);
	videobrush_.Close();
	target_.Root(nullptr);
	rootv_.Close();
	target_.Close();
	compositori_ = nullptr;
	compositor_.Close();
	SafeClose(&hcomposition_);
}

void VideoComponents::ReleaseDevice()
{
	dxgiMediaFactory_ = nullptr;
	dxgiFactory_ = nullptr;
	adapter_ = nullptr;
	dxgiDevice_ = nullptr;
	d3dDc_ = nullptr;
	d3dDevice_ = nullptr;
}

void VideoComponents::OnVideoArrive(ID3D11Texture2D* frames, UINT idx)
{
	D3D11_TEXTURE2D_DESC desc;
	backbuffer_->GetDesc(&desc);

	// We might be able to straight up present it without copy.
	// Windows 11 :
	// https://learn.microsoft.com/windows/win32/comp_swapchain/comp-swapchain-examples
	// Windows 10 :
	// Can we just use the surface from "IDXGIResource1::CreateSharedHandle" and bind it to visual?
	// Maybe the surface from the sharedhandle already has copy process.
	d3dDc_->CopySubresourceRegion1(
		backbuffer_.get(), 0,
		0, 0, 0,
		frames, idx, &vbox_, D3D11_COPY_DISCARD);

	videosc_->Present(0, DXGI_PRESENT_ALLOW_TEARING);
}
