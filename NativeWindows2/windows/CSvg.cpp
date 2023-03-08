#include <pch.h>
#include <NativeWindows2/windows/CSvg.h>
#include <NativeWindows2/windows/D2dWindow.h>

void CSvg::SetSvg(IStream* stream)
{
	ReleaseResources();

	stream_ = stream;
	HR(win_->bitmapdc_->CreateSvgDocument(
		stream,
		D2D1::SizeF(1.f, 1.f), 
		&doc_));

	doc_->GetRoot(&root_);
}

void CSvg::DrawSvg(int width, int height)
{
	if (doc_ == nullptr) return;

	surface_.Resize({ width, height });

	if (width < 1 || height < 1)
	{
		return;
	}

	auto dc = win_->BeginDraw1(surface_);
	dc->Clear();
	doc_->SetViewportSize(D2D1::SizeF((FLOAT)width, (FLOAT)height));
	dc->DrawSvgDocument(doc_);
	win_->EndDraw1(surface_);
}
