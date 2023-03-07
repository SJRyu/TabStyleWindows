#include <pch.h>
#include <NativeWindows2/windows/Scrollbar.h>
#include <NativeWindows2/windows/ScrollWindow.h>

Scrollbar::Scrollbar(WinArgs const& args, bool bHorizontal) :
	D2dWindow(args), bHorizontal_(bHorizontal)
{
	scrollwin_ = (ScrollWindow*)args.parent;
}

Scrollbar::~Scrollbar()
{

}

void Scrollbar::OnClose1()
{

}

LRESULT Scrollbar::OnCreate1(LPCREATESTRUCT createstr)
{
	visualb_ = AddColorVisual(Windows::UI::Colors::LightGray());

	LONG thick;
	if (bHorizontal_)
	{
		thick = rect_.height;
	}
	else
	{
		thick = rect_.width;
	}

	RECT rc{ 0, 0, thick, thick };

	ball_ = wmake_unique<Scrollball>(D2dWinArgs{ &rc, 0, this });
	ball_->CreateEx();
	ball_->ShowWindow();

	return 0;
}

LRESULT Scrollbar::OnSize(WPARAM state, int width, int height)
{
	LONG thick;
	if (bHorizontal_)
	{
		thick = height;
	}
	else
	{
		thick = width;
	}

	return 0;
}

LRESULT Scrollbar::OnLbtndown(int state, int x, int y)
{
	MoveBall(x, y);

	return 0;
}

LRESULT Scrollbar::OnLbtnup(int state, int x, int y)
{
	return 0;
}

LRESULT Scrollbar::UserMsgHandler(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{

	return ::DefWindowProc(hWnd, uMsg, wParam, lParam);
}

void Scrollbar::UpdateScrollbar()
{
	auto w = rect_.width;
	auto h = rect_.height;

	if (bHorizontal_)
	{
		auto barSize = (int)ceil((double)w * clientSize_ / pageSize_);
		auto barLoc = w * clientLoc_ / pageSize_;

		ball_->SetWindowPos(0,
			barLoc, 0, barSize, h, SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOREDRAW1);
	}
	else
	{
		auto barSize = (int)ceil((double)h * clientSize_ / pageSize_);
		auto barLoc = h * clientLoc_ / pageSize_;

		ball_->SetWindowPos(0,
			0, barLoc, w, barSize, SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOREDRAW1);
	}
}

void Scrollbar::MoveBall(int x, int y)
{
	int newcl;
	if (bHorizontal_)
	{
		newcl = x * pageSize_ / rect_.width;
	}
	else
	{
		newcl = y * pageSize_ / rect_.height;
	}
	auto rest = pageSize_ - clientSize_;
	newcl = std::min(rest, newcl);
	scrollwin_->OnScrollChanged(newcl, bHorizontal_);
}

/****************************************************************************************/

Scrollball::Scrollball(D2dWinArgs const& args) :
	D2dWindow1(args)
{
	bar_ = (Scrollbar*)args.parent;
	bHorizontal_ = bar_->IsHorizontal();
}

Scrollball::~Scrollball()
{

}

LRESULT Scrollball::OnCreate1(LPCREATESTRUCT createstr)
{
	visualb_ = AddColorVisual(Windows::UI::Colors::AliceBlue());

	return 0;
}

void Scrollball::OnClose1()
{

}

LRESULT Scrollball::OnMousemove(int state, int x, int y)
{
	if (bDrag_)
	{
		int nx = rect_.left;
		int ny = rect_.top;
		if (bHorizontal_)
		{
			int dx = x - capturex_;
			nx = std::max((int)rect_.left + dx, 0);
			nx = std::min((int)bar_->rect_.width, nx);
		}
		else
		{
			int dy = y - capturey_;
			ny = std::max((int)rect_.top + dy, 0);
			ny = std::min((int)bar_->rect_.height, ny);
		}

		bar_->MoveBall(nx, ny);
	}
	return 0;
}

LRESULT Scrollball::OnMove(int x, int y)
{
	return 0;
}

LRESULT Scrollball::OnSize(WPARAM state, int width, int height)
{
	return 0;
}

LRESULT Scrollball::OnLbtndown(int state, int x, int y)
{
	bDrag_ = true;
	::SetCapture(hwnd_);

	capturex_ = x;
	capturey_ = y;

	return 0;
}

LRESULT Scrollball::OnLbtnup(int state, int x, int y)
{
	bDrag_ = false;
	::ReleaseCapture();

	return 0;
}
