#include <pch.h>
#include <NativeWindows2/windows/VideoWindow.h>

VideoWindow::VideoWindow(WinArgs const& args) :
	Win32Window(args), VideoComponents(this)
{
	assert(parent_ != nullptr);
	//wstyle_ |= WS_CLIPSIBLINGS;
}

VideoWindow::~VideoWindow()
{

}

LRESULT VideoWindow::OnCreate(LPCREATESTRUCT createstr)
{
	CreateDxResources();

	return OnCreate1(createstr);
}

void VideoWindow::OnClose()
{
	OnClose1();
#ifdef WINRT_CLOSETEST
	ReleaseDxResources();
#endif
}
