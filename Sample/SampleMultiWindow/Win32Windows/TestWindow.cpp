#include <pch.h>
#include <Win32Windows/TestWindow.h>
#include <NativeWindows2/windows/ClientWindow.h>
#include <Win32Windows/TestTab.h>

using namespace NativeWindows;

TestWindow::TestWindow(TestTab* tab) :
	Win32Window({ 0, 0, tab->client_.get()}), tab_(tab)
{
	wstyle_ |= WS_VISIBLE;
}

TestWindow::~TestWindow()
{
	dbg_msg("tw deeleted");
}

VOID TestWindow::OnClose()
{
	dbg_msg("tw closed");
}

LRESULT TestWindow::OnCreate(LPCREATESTRUCT createstr)
{

	return 0;
}

LRESULT TestWindow::OnSize(WPARAM state, int width, int height)
{

	return 0;
}

