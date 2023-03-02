#include <pch.h>
#include <NativeWindows2/windows/CSvg.h>
#include <NativeWindows2/windows/D2dWindow.h>

void CSvg::SetSvg(IStream* stream)
{
	ReleaseResources();

	stream_ = stream;
	HR(win_->bitmapdc_->CreateSvgDocument(
		stream,
		D2D1::SizeF(1.f, 1.f), //not effect, but viewport-size
		&doc_));

	doc_->GetRoot(&root_);
}

void CSvg::DrawSvg()
{
	if (doc_ == nullptr) return;

	auto ssize = surface_.Size();
	if (ssize.Width < 1 || ssize.Height < 1)
	{
		return;
	}

	auto dc = win_->BeginDraw1(surface_);
	dc->Clear();
	// is this just a transform after drawing? i hope not.
	doc_->SetViewportSize(D2D1::SizeF((FLOAT)ssize.Width, (FLOAT)ssize.Height));
	dc->DrawSvgDocument(doc_);
	win_->EndDraw1(surface_);
}
