#include <pch.h>
#include <NativeWindows2/directx/D2dComponents.h>
#include <NativeWindows2/windows/Win32Window.h>
#include <NativeWindows2/Win32UIThread.h>

D2dComponents::D2dComponents(Win32Window* window) :
	window_(window)
{
};

D2dComponents::~D2dComponents()
{
};

void D2dComponents::InitCompositor()
{
	refres_ = window_->thread_->Res();
	target_ = CreateDesktopWindowTarget(refres_->compositor_, window_->hwnd_);

	rootv_ = refres_->compositor_.CreateContainerVisual();
	rootv_.RelativeSizeAdjustment({ 1.0f, 1.0f });
	target_.Root(rootv_);

	topv_ = refres_->compositor_.CreateContainerVisual();
	topv_.RelativeSizeAdjustment({ 1.0f, 1.0f });
	bottomv_ = refres_->compositor_.CreateContainerVisual();
	bottomv_.RelativeSizeAdjustment({ 1.0f, 1.0f });

	rootv_.Children().InsertAtTop(topv_);
	rootv_.Children().InsertAtBottom(bottomv_);

	visuals_ = bottomv_.Children();
}

void D2dComponents::ReleaseCompositor()
{
	target_.Root(nullptr);
	rootv_.Close();
	target_.Close();
}

void D2dComponents::InitCompositor1(D2dComponents* parent)
{
	refres_ = window_->thread_->Res();
	target_ = parent->target_;

	CRECT<FLOAT> rect = window_->rect_;
	rootv_ = refres_->compositor_.CreateContainerVisual();
	rootv_.Offset({ rect.left, rect.top, 0 });
	rootv_.Size({ rect.width, rect.height });

	auto clip = refres_->compositor_.CreateInsetClip(
		0, 0, 0, 0);
	rootv_.Clip(clip);

	parentv_ = parent->topv_.Children();

	//InsertAsync(parentv_, rootv_).get();
	parentv_.InsertAtTop(rootv_);

	topv_ = refres_->compositor_.CreateContainerVisual();
	topv_.RelativeSizeAdjustment({ 1.0f, 1.0f });
	bottomv_ = refres_->compositor_.CreateContainerVisual();
	bottomv_.RelativeSizeAdjustment({ 1.0f, 1.0f });

	rootv_.Children().InsertAtTop(topv_);
	rootv_.Children().InsertAtBottom(bottomv_);

	visuals_ = bottomv_.Children();

	using timespan = std::chrono::duration<int, std::ratio<1, 1000>>;
	animemove_ = refres_->compositor_.CreateVector3KeyFrameAnimation();
	animemove_.Duration(timespan(400));

	animesize_ = refres_->compositor_.CreateVector2KeyFrameAnimation();
	animesize_.Duration(timespan(400));
}

void D2dComponents::ReleaseCompositor1()
{
	animesize_.Close();
	animemove_.Close();

	parentv_.Remove(rootv_);
	parentv_ = nullptr;
	rootv_.Close();
}

void D2dComponents::CreateBitmapDc()
{
	HR(refres_->d2dDevice_->CreateDeviceContext(
		D2D1_DEVICE_CONTEXT_OPTIONS_NONE, bitmapdc_.put()));

	bmprops_.dpiX = 0;
	bmprops_.dpiY = 0;
	bmprops_.pixelFormat.alphaMode = D2D1_ALPHA_MODE_PREMULTIPLIED;
	bmprops_.pixelFormat.format = DXGI_FORMAT_B8G8R8A8_UNORM;
	bmprops_.bitmapOptions = D2D1_BITMAP_OPTIONS_TARGET | D2D1_BITMAP_OPTIONS_CANNOT_DRAW;
	bmprops_.colorContext = nullptr;
}
