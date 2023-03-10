#include <pch.h>
#include <Win32Windows/NvrWindow.h>
#include <Win32Windows/NvrTab.h>
#include <NativeWindows2/windows/ClientWindow.h>
#include <Win32Windows/VideoView.h>
#include <Win32Windows/TestWindow1.h>

using namespace NativeWindows;

NvrWindow::NvrWindow(NvrTab* tab) : 
	tab_(tab)
{
	wstyle_ |= WS_VISIBLE;
}

NvrWindow::~NvrWindow()
{
}

VOID NvrWindow::OnClose()
{
	svgwin_.reset();
	std::fill(views_.begin(), views_.end(), nullptr);
}

LRESULT NvrWindow::OnCreate(LPCREATESTRUCT createstr)
{
	int x, y;
	int cw = rect_.width / CAMS_IN_ROW;
	int ch = rect_.height / CAMS_IN_ROW;
	for (int pos = 0; pos < MAX_CAM_COUNT; pos++)
	{
		x = (pos % CAMS_IN_ROW) * cw;
		y = (pos / CAMS_IN_ROW) * ch;

		RECT rect{ x, y, x + cw, y + ch };

		views_[pos] = wmake_unique<VideoView>(
			std::forward<WinArgs>({ &rect, 0, (Win32Window*)this }), pos, pos);
		views_[pos]->CreateEx();
	}

	x = rect_.width / 2;
	y = rect_.height / 2;
	int cx = DpiVal(120), cy = DpiVal(120);
	x -= cx / 2;
	y -= cy / 2;
	RECT rect{ x, y, x + cx, y + cy };
	svgwin_ = wmake_unique<TestWindow1>(WinArgs{ &rect, 0, this });
	svgwin_->CreateEx();
	svgwin_->ShowWindow();

	return 0;
}

LRESULT NvrWindow::OnSize(WPARAM state, int width, int height)
{
	int x, y;
	int cw = width / CAMS_IN_ROW;
	int ch = height / CAMS_IN_ROW;
	for (int pos = 0; pos < MAX_CAM_COUNT; pos++)
	{
		x = (pos % CAMS_IN_ROW) * cw;
		y = (pos / CAMS_IN_ROW) * ch;

		views_[pos]->SetWindowPos(0,
			x, y, cw, ch,
			SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOREDRAW1);
	}

	x = rect_.width / 2;
	y = rect_.height / 2;
	int cx = DpiVal(120), cy = DpiVal(120);
	x -= cx / 2;
	y -= cy / 2;
	svgwin_->SetWindowPos(HWND_TOP,
		x, y, cx, cy,
		SWP_NOACTIVATE | SWP_NOREDRAW1);

	return 0;
}

void NvrWindow::CalcChildRect(int pos, CRECT<LONG>* outref)
{
	int cw = rect_.right / CAMS_IN_ROW;
	int ch = rect_.height / CAMS_IN_ROW;

	int x = (pos % CAMS_IN_ROW) * cw;
	int y = (pos / CAMS_IN_ROW) * ch;

	outref->left = x;
	outref->top = y;
	outref->width = cw;
	outref->height = ch;
}
