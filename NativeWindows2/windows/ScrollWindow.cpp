#include <pch.h>
#include <NativeWindows2/windows/ScrollWindow.h>
#include <NativeWindows2/MainLoop.h>

ScrollWindow::ScrollWindow(WinArgs const& args) :
	Win32Window(args)
{
	ml_ = (MainLoop*)root_->thread_;
}

ScrollWindow::~ScrollWindow()
{

}

void ScrollWindow::OnClose()
{
	//probably we don't need this 
	ml_->ClearVbiMsg(hwnd_);
	OnClose1();
}

LRESULT ScrollWindow::OnCreate(LPCREATESTRUCT createstr)
{
	return OnCreate1(createstr);
}

LRESULT ScrollWindow::OnSize(WPARAM state, int width, int height)
{
	UpdateScroll();
	return OnSize1(state, width, height);
}
BOOL ScrollWindow::OnMouseWheel(WPARAM wp, LPARAM lp)
{
	//WHEEL_DELTA
	auto delta = GET_WHEEL_DELTA_WPARAM(wp);
	auto key = GET_KEYSTATE_WPARAM(wp);

	if (key == MK_SHIFT)
	{
		if (delta > 0)
		{
			ml_->QueueVbiMsg(Win32MsgArgs{ hwnd_, UM_HSCROLL_UP, 0, 0 });
		}
		else
		{
			ml_->QueueVbiMsg(Win32MsgArgs{ hwnd_, UM_HSCROLL_DOWN, 0, 0 });
		}
	}
	else
	{
		if (delta > 0)
		{
			ml_->QueueVbiMsg(Win32MsgArgs{ hwnd_, UM_VSCROLL_UP, 0, 0 });
		}
		else
		{
			ml_->QueueVbiMsg(Win32MsgArgs{ hwnd_, UM_VSCROLL_DOWN, 0, 0 });
		}
	}

	return TRUE;
}

LRESULT ScrollWindow::UserMsgHandler(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case UM_VSCROLL_UP:
	{
		VScrollup();
		break;
	}
	case UM_VSCROLL_DOWN:
	{	
		VScrolldown();
		break;
	}
	case UM_HSCROLL_UP:
	{
		HScrollup();
		break;
	}
	case UM_HSCROLL_DOWN:
	{
		HScrolldown();
		break;
	}
	case UM_CHILD_SIZE:
	{
		UpdateScroll();
		break;
	}
	case UM_CHILD_MOVE:
	{
		UpdateScroll();
		break;
	}
	default :
		return ::DefWindowProc(hWnd, uMsg, wParam, lParam);
	}

	return 0;
}

void ScrollWindow::UpdateScroll()
{
	//update scrollbar
}

bool ScrollWindow::VScrollup()
{
	auto ttop = target_->rect_.top;
	if (ttop >= 0) return false;

	auto dy = DpiVal(SCROLLUNITPIXEL);
	dy = std::min(dy, -ttop);

	// this freezing message loop, so,
	// probably using a VBI task is better.
	DwmFlush(); 
	target_->MoveWindow(target_->rect_.left, ttop + dy);

	return true;
}

bool ScrollWindow::VScrolldown()
{
	auto h = rect_.height;
	auto ttop = target_->rect_.top;
	auto tbottom = target_->rect_.bottom;
	if (tbottom <= h) return false;

	auto dy = -DpiVal(SCROLLUNITPIXEL);
	auto rest = h - tbottom;
	dy = std::max(dy, rest);

	// this freezing message loop, so,
	// probably using a VBI task is better.
	DwmFlush(); 
	target_->MoveWindow(target_->rect_.left, ttop + dy);

	return true;
}

bool ScrollWindow::HScrollup()
{
	auto tleft = target_->rect_.left;
	if (tleft >= 0) return false;

	auto dx = DpiVal(SCROLLUNITPIXEL);
	dx = std::min(dx, -tleft);

	DwmFlush();
	target_->MoveWindow(tleft + dx, target_->rect_.top);

	return true;
}

bool ScrollWindow::HScrolldown()
{
	auto w = rect_.width;
	auto tleft = target_->rect_.left;
	auto tright = target_->rect_.right;
	if (tright <= w) return false;

	auto dx = -DpiVal(SCROLLUNITPIXEL);
	auto rest = w - tright;
	dx = std::max(dx, rest);

	DwmFlush();
	target_->MoveWindow(tleft + dx, target_->rect_.top);

	return true;
}
