#include <pch.h>
#include <NativeWindows2/windows/ScrollWindow.h>
#include <NativeWindows2/windows/Scrollbar.h>
#include <NativeWindows2/MainLoop.h>

ScrollWindow::ScrollWindow(WinArgs const& args) :
	D2dWindow(args)
{
	ml_ = MainLoop::GetInstance();
}

ScrollWindow::~ScrollWindow()
{

}

void ScrollWindow::OnClose()
{
	vscroll_.reset();
	notch_.reset();
	ml_->ClearVbiMsg(hwnd_); //probably we don't need this 
	OnClose1();
}

LRESULT ScrollWindow::OnCreate(LPCREATESTRUCT createstr)
{
	InitCompositor();

	auto w = rect_.width;
	auto h = rect_.height;
	clientRect_ = { 0, 0, w, h };

	auto thick = DpiVal(SCROLLBARTHICK);
	RECT rc{ 0, 0, thick, thick };
	notch_ = wmake_unique<ScrollNotch>(WinArgs{&rc, 0, this});
	notch_->CreateEx();
	notch_->SetWindowPos(HWND_TOP, 0, 0, 0, 0,
		SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE | SWP_NOREDRAW1);

	rc = { 0, thick, thick, h };
	vscroll_ = wmake_unique<Scrollbar>(WinArgs{ &rc, 0, this });
	vscroll_->CreateEx();
	vscroll_->SetWindowPos(HWND_TOP, 0, 0, 0, 0, 
		SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE | SWP_NOREDRAW1);

	rc = { thick, 0, w, thick };
	hscroll_ = wmake_unique<Scrollbar>(WinArgs{ &rc, 0, this }, true);
	hscroll_->CreateEx();
	hscroll_->SetWindowPos(HWND_TOP, 0, 0, 0, 0,
		SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE | SWP_NOREDRAW1);

	return OnCreate1(createstr);
}

LRESULT ScrollWindow::OnSize(WPARAM state, int width, int height)
{
	auto thick = DpiVal(SCROLLBARTHICK);
	vscroll_->SetWindowPos(0, 
		0, thick, thick, height - thick,
		SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOREDRAW1);

	hscroll_->SetWindowPos(0,
		thick, 0, width - thick, thick,
		SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOREDRAW1);

	clientRect_.right = width;
	clientRect_.bottom = height;

	//dbg_msg("onsize thick = %d", thick);

	UpdateScroll();
	UpdateScrollbar();
	return OnSize1(state, width, height);
}

LRESULT ScrollWindow::OnDpichangedAfterparent()
{
	auto thick = DpiVal(SCROLLBARTHICK);

	notch_->SetWindowPos(0,
		0, 0, thick, thick,
		SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOREDRAW1);

	if (isVScrolling_)
		clientRect_.left = thick;
	if (isHScrolling_)
		clientRect_.top = thick;

	UpdateScroll();
	UpdateScrollbar();

	return 0;
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
	case UM_SCROLLCHANGED:
	{
		OnScrollChanged_((int)wParam, (bool)lParam);
		break;
	}
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
		UpdateScrollbar();
		break;
	}
	case UM_CHILD_MOVE:
	{
		UpdateScroll();
		UpdateScrollbar();
		break;
	}
	default :
		return ::DefWindowProc(hWnd, uMsg, wParam, lParam);
	}

	return 0;
}

int ScrollWindow::ScrollMinX()
{
	return (rect_.width - target_->rect_.width);
}

int ScrollWindow::ScrollMaxX()
{
	int ret = 0;
	if (isVScrolling_)
	{
		ret = vscroll_->rect_.right;
	}
	return ret;
}

int ScrollWindow::ScrollMinY()
{
	return (rect_.height - target_->rect_.height);
}

int ScrollWindow::ScrollMaxY()
{
	int ret = 0;
	if (isHScrolling_)
	{
		ret = hscroll_->rect_.bottom;
	}
	return ret;
}

void ScrollWindow::UpdateScrollbar()
{
	auto trect = target_->rect_;
	ScrollInfo info;

	if (isHScrolling_)
	{
		info.pageSize_ = trect.width;
		info.clientSize_ = clientRect_.width;
		info.clientLoc_ = clientRect_.left - trect.left;

		hscroll_->UpdateScrollbar(info);
	}

	if (isVScrolling_)
	{
		info.pageSize_ = trect.height;
		info.clientSize_ = clientRect_.height;
		info.clientLoc_ = clientRect_.top - trect.top;

		vscroll_->UpdateScrollbar(info);
	}
}

void ScrollWindow::UpdateScroll()
{
	//update scrollbar
	auto trect = target_->rect_;
	auto tw = trect.width;
	auto th = trect.height;

	auto w = rect_.width;
	auto h = rect_.height;
	auto thick = DpiVal(SCROLLBARTHICK);

	POINT pt{ trect.left, trect.top };
	LONG dy = 0, dx = 0;

	if (th > clientRect_.height)
	{
		if (isVScrolling_ == false)
		{
			dx = thick;
			clientRect_.left = thick;
			vscroll_->ShowWindow();
			isVScrolling_ = true;
		}
	}
	else
	{
		if (isVScrolling_)
		{
			clientRect_.left = 0;
			vscroll_->ShowWindow(SW_HIDE);
			isVScrolling_ = false;
		}
	}

	if (tw > clientRect_.width)
	{
		if (isHScrolling_ == false)
		{
			dy = thick;
			clientRect_.top = thick;
			hscroll_->ShowWindow();
			isHScrolling_ = true;
		}
	}
	else
	{
		if (isHScrolling_)
		{
			clientRect_.top = 0;
			hscroll_->ShowWindow(SW_HIDE);
			isHScrolling_ = false;
		}
	}

	//dbg_msg("client = %d, %d | trect = %d, %d", 
		//clientRect_.left, clientRect_.top, trect.left, trect.top);

	if (trect.bottom < clientRect_.bottom)
	{
		dy = clientRect_.bottom - trect.bottom;
	}

	if (trect.top > clientRect_.top)
	{
		dy = clientRect_.top - trect.top;
	}

	if (trect.right < clientRect_.right)
	{
		dx = clientRect_.right - trect.right;
	}

	if (trect.left > clientRect_.left)
	{
		dx = clientRect_.left - trect.left;
	}

	if (dx || dy)
	{
		pt.x += dx;
		pt.y += dy;
		target_->MoveWindowAsync(pt.x, pt.y);
	}

	if (isVScrolling_ || isHScrolling_)
	{
		notch_->ShowWindow(SW_SHOW);
	}
	else
	{
		notch_->ShowWindow(SW_HIDE);
	}
}

void ScrollWindow::OnScrollChanged(int newcl, bool bHorizontal)
{
	ml_->QueueVbiMsg(Win32MsgArgs{ hwnd_, UM_SCROLLCHANGED,
			(WPARAM)newcl, (LPARAM)bHorizontal });
}

void ScrollWindow::OnScrollChanged_(int newcl, bool bHorizontal)
{
	int pageloc;
	if (bHorizontal)
	{
		pageloc = clientRect_.left - newcl;
		//dbg_msg("pageloc = %d", pageloc);
		target_->MoveWindowAsync(pageloc, target_->rect_.top);
	}
	else
	{
		pageloc = clientRect_.top - newcl;
		//dbg_msg("pageloc = %d", pageloc);
		target_->MoveWindowAsync(target_->rect_.left, pageloc);
	}
}

void ScrollWindow::SetTarget(Win32Window* target)
{
	target_ = target;
	target->bScroll_ = true;
	UpdateScroll();
	UpdateScrollbar();
	target_->MoveWindowAsync(clientRect_.left, clientRect_.top);
}

bool ScrollWindow::VScrollup()
{
	if (isVScrolling_ == false) return false;
	auto trect = target_->rect_;

	auto dy = DpiVal(SCROLLUNITPIXEL);
	
	if (isHScrolling_)
	{
		dy = std::min(dy, DpiVal(SCROLLBARTHICK) - trect.top);
	}
	else
	{
		dy = std::min(dy, -trect.top);
	}
	target_->MoveWindowAsync(trect.left, trect.top + dy);

	return true;
}

bool ScrollWindow::VScrolldown()
{
	if (isVScrolling_ == false) return false;
	auto trect = target_->rect_;

	auto h = rect_.height;

	auto dy = -DpiVal(SCROLLUNITPIXEL);
	auto rest = h - trect.bottom;
	dy = std::max(dy, rest);

	target_->MoveWindowAsync(trect.left, trect.top + dy);

	return true;
}

bool ScrollWindow::HScrollup()
{
	if (isHScrolling_ == false) return false;
	auto trect = target_->rect_;

	auto dx = DpiVal(SCROLLUNITPIXEL);
	if (isVScrolling_)
	{
		dx = std::min(dx, DpiVal(SCROLLBARTHICK) - trect.left);
	}
	else
	{
		dx = std::min(dx, -trect.left);
	}
	target_->MoveWindowAsync(trect.left + dx, trect.top);

	return true;
}

bool ScrollWindow::HScrolldown()
{
	if (isHScrolling_ == false) return false;
	auto trect = target_->rect_;

	auto w = rect_.width;

	auto dx = -DpiVal(SCROLLUNITPIXEL);
	auto rest = w - trect.right;
	dx = std::max(dx, rest);

	target_->MoveWindowAsync(trect.left + dx, trect.top);

	return true;
}

/*********************************************************************/

ScrollNotch::ScrollNotch(WinArgs const& args) :
	D2dWindow(args)
{

}

ScrollNotch::~ScrollNotch()
{

}

void ScrollNotch::OnClose1()
{

}

LRESULT ScrollNotch::OnCreate1(LPCREATESTRUCT createstr)
{
	visualb_ = AddColorVisual(Windows::UI::Colors::AliceBlue());
	return 0;
}

LRESULT ScrollNotch::OnLbtndown(int state, int x, int y)
{
	return 0;
}

LRESULT ScrollNotch::UserMsgHandler(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	return ::DefWindowProc(hWnd, uMsg, wParam, lParam);
}
