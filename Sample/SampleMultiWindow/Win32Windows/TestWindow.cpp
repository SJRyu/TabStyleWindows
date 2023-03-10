#include <pch.h>
#include <Win32Windows/TestWindow.h>
#include <NativeWindows2/windows/ClientWindow.h>
#include <Win32Windows/TestTab.h>

using namespace NativeWindows;

TestWindow::TestWindow(TestTab* tab) : tab_(tab)
{
}

TestWindow::~TestWindow()
{
}

VOID TestWindow::OnClose1()
{
}

LRESULT TestWindow::OnCreate1(LPCREATESTRUCT createstr)
{

	return 0;
}

LRESULT TestWindow::OnSize(WPARAM state, int width, int height)
{

	return 0;
}

