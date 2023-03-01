#include <pch.h>
#include <NativeWindows2/MainLoop.h>
#include <NativeWindows2/windows/ContainerWindow.h>
#include <NativeWindows2/windows/Ctab.h>

ContainerWindow::ContainerWindow(
	RECT* rect, Win32UIThread* thread, int idx, LONG maxtab) :
	Win32Window({ rect, 0, 0, thread }), idx_(idx), tabmaxn_(maxtab), 
	ml_((MainLoop*)thread_)
{
	wstyle_ |= WS_OVERLAPPEDWINDOW | WS_VISIBLE;
	wstylex_ |= WS_EX_NOREDIRECTIONBITMAP;

	ctab_ = std::make_unique<Ctab>(this);
}

ContainerWindow::~ContainerWindow()
{
	ctab_.reset();
}

void ContainerWindow::OnClose()
{
	ctab_->WaitClose();
	PostThreadMessageW(thread_->threadid_, UTM_CLOSECONTAINER, (WPARAM)this, 0);
}

LRESULT ContainerWindow::OnCreate(LPCREATESTRUCT createstr)
{
	DWMNCRENDERINGPOLICY policy = DWMNCRP_ENABLED;
	HR(DwmSetWindowAttribute(hwnd_, DWMWA_NCRENDERING_POLICY, &policy, sizeof(policy)));
	BOOL enable = false;
	HR(DwmSetWindowAttribute(hwnd_, DWMWA_ALLOW_NCPAINT, &enable, sizeof(enable)));

	MARGINS margin{ 0, 0, ncm_->captionh, 0 };
	HR(DwmExtendFrameIntoClientArea(hwnd_, &margin));

	xRect_ = rect_;
	cxmin_ = DpiVal(containerMinWidth_);
	cymin_ = ncm_->captionh + ncm_->hframe;

	ctab_->CreateEx();

	SetWindowPos(HWND_BOTTOM, rect_.left, rect_.top, rect_.width, rect_.height,
		SWP_NOACTIVATE | SWP_FRAMECHANGED);

	return 0;
}

void ContainerWindow::OnDpichanged(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	MARGINS margin{ 0, 0, ncm_->captionh, 0 };
	HR(DwmExtendFrameIntoClientArea(hwnd_, &margin));

	cxmin_ = DpiVal(containerMinWidth_);
	cymin_ = ncm_->captionh + ncm_->hframe;

	assert(lParam);
	RECT* nrc = (RECT*)lParam;

	SetWindowPos(0, nrc->left, nrc->top, nrc->right - nrc->left, nrc->bottom - nrc->top,
		SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOREDRAW1);
}


LRESULT ContainerWindow::OnSize(WPARAM state, int width, int height)
{
	if (state == SIZE_RESTORED)
	{
		xRect_ = rect_;
	}

	ctab_->PostResize(isSizing_);

	//::RedrawWindow(hwnd_, 0, 0, RDW_NOINTERNALPAINT | RDW_NOCHILDREN);
	ValidateRect(hwnd_, nullptr);
	return 0;
}

LRESULT ContainerWindow::OnExitSizemove()
{
	ctab_->PostResize();
	return 0;
}

LRESULT ContainerWindow::OnMove(int x, int y)
{
	if (isDragbyTab_)
	{
		isFindingDoc_ = true;
		auto dock = ml_->FindDock(x, y);
		isFindingDoc_ = false;
		if (dock)
		{
			//dbg_msg("found dock %llx", (LONG_PTR)dock);
			auto tab = ctab_->tab_;
			ctab_->MoveoutTab(tab);
			dock->ctab_->InsertTab(tab);
			::mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
		}
	}
	return 0;
}

LRESULT ContainerWindow::OnNcHitTest(LPARAM lparam)
{
	if (isFindingDoc_) return HTTRANSPARENT;

	int x = GET_X_LPARAM(lparam);
	int y = GET_Y_LPARAM(lparam);

	RECT rect;
	GetWindowRect(hwnd_, &rect);

	USHORT uRow = 1;
	USHORT uCol = 1;

	bool bCaption = false;

	int hborder = ncm_->hborder;
	int hframe = ncm_->hframe;
	int vframe = ncm_->vframe;
	int captionh = ncm_->captionh;

	if (y <= rect.top + hborder)
	{
		uRow = 0;
	}
	else
	{
		if (y < rect.top + captionh)
		{
			isDragbyTab_ = false;
			bCaption =  true;
		}
		else if (y > rect.bottom - hframe)
		{
			uRow = 2;
		}
	}

	if (x < rect.left + vframe)
	{
		uCol = 0; 
	}
	else if (x > rect.right - vframe)
	{
		uCol = 2; 
	}

	LRESULT hitTests[3][3] =
	{
		{ HTTOPLEFT,    HTTOP,    HTTOPRIGHT },
		{ HTLEFT,       bCaption ? HTCAPTION : HTCLIENT,     HTRIGHT },
		{ HTBOTTOMLEFT, HTBOTTOM, HTBOTTOMRIGHT },
	};

	return hitTests[uRow][uCol];
}

LRESULT ContainerWindow::UserMsgHandler(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_NCCALCSIZE:
	{
		auto& rectw = ((NCCALCSIZE_PARAMS*)lParam)->rgrc[0];
		auto pos = ((NCCALCSIZE_PARAMS*)lParam)->lppos;
		if (wParam)
		{
			rectw.right -= ncm_->vframe;
			rectw.left += ncm_->vframe;
			rectw.bottom -= ncm_->hframe;
		}
		return 0;
	}
	case WM_GETMINMAXINFO:
	{
		LPMINMAXINFO lpMMI = (LPMINMAXINFO)lParam;

		lpMMI->ptMinTrackSize.x = cxmin_;
		lpMMI->ptMinTrackSize.y = cymin_;
		return 0;
	}
	default:
		break;
	}
	
	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

