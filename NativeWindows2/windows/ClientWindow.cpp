#include <pch.h>
#include <NativeWindows2/windows/ClientWindow.h>
#include <NativeWindows2/windows/TabWindow.h>
#include <NativeWindows2/windows/Ctab.h>
#include <NativeWindows2/windows/ContainerWindow.h>
#include <NativeWindows2/Win32UIThread.h>
#include <NativeWindows2/windows/Scrollbar.h>

ClientWindow::ClientWindow(TabWindow* tab) :
	D2dWindow({ 0, 0, tab->refcontainer_ }), 
	tab_(tab), container_(tab->refcontainer_)
{

}

ClientWindow::~ClientWindow()
{

}

void ClientWindow::CreateEx()
{
	rect_.left = 0;
	rect_.right = container_->rect_.width;
	rect_.top = ncm_->captionh;
	rect_.bottom = container_->rect_.height - rect_.top;
	Win32Window::CreateEx();
}

HWND ClientWindow::SetParent(Win32Window* parent)
{
	auto ret = Win32Window::SetParent(parent);
	if (ret != NULL)
	{
		container_ = (ContainerWindow*)parent;
	}
	return ret;
}

void ClientWindow::OnClose1()
{
	scwin_.reset();
	cthread_.reset();
}

LRESULT ClientWindow::OnCreate1(LPCREATESTRUCT createstr)
{
	visualbg_ = AddColorVisual(Windows::UI::Colors::White());

	cthread_ = std::make_unique<Win32UIThread>();
	cthread_->Start();

	RECT rc = { 0, 0, rect_.width, rect_.height };
	scwin_ = wmake_unique<ClientScroll>(WinArgs{ &rc, 0, this, cthread_.get() });
	scwin_->SetTarget(content_);
	scwin_->CreateEx1();
	scwin_->ShowWindow();
	
	return 0;
}

LRESULT ClientWindow::OnSize(WPARAM state, int width, int height)
{
	::PostMessage(scwin_->hwnd_, UM_CLIENT_RESIZE, (WPARAM)width, (LPARAM)height);
	return 0;
}

LRESULT ClientScroll::OnSize(WPARAM state, int width, int height)
{
	clientRect_.right = width;
	clientRect_.bottom = height;

	auto thick = DpiVal(SCROLLBARTHICK);
	vscroll_->SetWindowPos(0,
		0, thick, thick, height - thick,
		SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOREDRAW1);

	hscroll_->SetWindowPos(0,
		thick, 0, width - thick, thick,
		SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOREDRAW1);

	target_->SetWindowPos(0,
		0, 0, width, height,
		SWP_NOZORDER | SWP_NOMOVE | SWP_NOACTIVATE | SWP_NOREDRAW1);

	UpdateScroll();
	UpdateScrollbar();

	return 0;
}

void ClientWindow1::OnClose1()
{
	content_->WaitClose();
	SafeDelete(&content_);
}

LRESULT ClientWindow1::OnCreate1(LPCREATESTRUCT createstr)
{
	visualbg_ = AddColorVisual(Windows::UI::Colors::White());

	RECT rc{ 0, 0, rect_.width, rect_.height };
	content_->SetWindowArgs(WinArgs{ &rc, 0, this, content_->thread_ });
	content_->CreateEx1();

	return 0;
}

LRESULT ClientWindow1::OnSize(WPARAM state, int width, int height)
{
	::PostMessage(content_->hwnd_, UM_CLIENT_RESIZE, (WPARAM)width, (LPARAM)height);
	return 0;
}
