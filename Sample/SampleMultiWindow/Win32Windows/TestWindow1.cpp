#include <pch.h>
#include <Win32Windows/TestWindow1.h>
#include <AssetManager.h>
#include <NativeWindows2/windows/CSvg.h>

using namespace NativeWindows;

static inline void SafeDeleteObject(HGDIOBJ* obj)
{
	if (*obj)
	{
		::DeleteObject(*obj);
		*obj = nullptr;
	}
}

TestWindow1::TestWindow1(WinArgs const& args) :
	D2dWindow(args)
{

}

TestWindow1::~TestWindow1()
{
	
}

LRESULT TestWindow1::OnCreate1(LPCREATESTRUCT createstr)
{
	CreateBitmapDc();

	svgvisual_ = AddD2dVisual(svgsurface_, rect_.width, rect_.height);

	IStream* stream = nullptr; 
	AssetManager::GetStream(AssetManager::ASSET_TEST, &stream);
	csvg_ = std::make_unique<CSvg>(this, svgsurface_, stream);

	return 0;
}

LRESULT TestWindow1::OnSize(WPARAM state, int width, int height)
{
	csvg_->DrawSvg(width, height);
	UpdateRegion();
	return 0;
}

LRESULT TestWindow1::OnLbtndown(int state, int x, int y)
{
	::SendMessage(hwnd_, WM_SYSCOMMAND, SC_DRAGMOVE, 0);
	return 0;
}

VOID TestWindow1::OnClose1()
{
}

void TestWindow1::UpdateRegion()
{
	/*
	After a successful call to SetWindowRgn, the system owns the region specified by the region handle hRgn.
	The system does not make a copy of the region. Thus, you should not make any further function calls with this region handle.
	In particular, do not delete this region handle. The system deletes the region handle when it no longer needed.
	*/
	auto region = ::CreateEllipticRgn(0, 0, rect_.width, rect_.height);
	::SetWindowRgn(hwnd_, region, false);
}