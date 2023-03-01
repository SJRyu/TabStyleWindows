#include <pch.h>
#include <NativeWindows2/Win32UIThread.h>
#include <NativeWindows2/windows/ContainerWindow.h>
#include <NativeWindows2/windows/Ctab.h>

ContainerWindow::ContainerWindow(RECT* rect, Win32UIThread* thread) :
	D2dWindow({ rect, 0, 0, thread })
{
	wstyle_ = WS_OVERLAPPEDWINDOW | WS_VISIBLE;
	wstylex_ = WS_EX_NOREDIRECTIONBITMAP;
}

ContainerWindow::~ContainerWindow()
{
	SafeDelete(&ctab_);
}

void ContainerWindow::Close()
{
	PostThreadMessageW(thread_->threadid_, UTM_CLOSECONTAINER, (WPARAM)this, 0);
}

void ContainerWindow::OnDestroy()
{
	//PostQuitMessage(0);
	Close();
}

LRESULT ContainerWindow::OnCreate1(LPCREATESTRUCT createstr)
{
#if 1
	DWMNCRENDERINGPOLICY policy = DWMNCRP_ENABLED;
	HR(DwmSetWindowAttribute(hwnd_, DWMWA_NCRENDERING_POLICY, &policy, sizeof(policy)));
	BOOL enable = false;
	HR(DwmSetWindowAttribute(hwnd_, DWMWA_ALLOW_NCPAINT, &enable, sizeof(enable)));
#endif

	MARGINS margin{ 0, 0, ncm_->captionh, 0 };
	HR(DwmExtendFrameIntoClientArea(hwnd_, &margin));

	cxmin_ = Valfordpi(containerMinWidth_);
	cymin_ = ncm_->captionh + ncm_->hframe;

	//AddColorVisual({ 0xFF, 0xFF, 0xFF, 0xFF });

	ctab_ = new Ctab(this);

	SetWindowPos(HWND_BOTTOM, rect_.left, rect_.top, rect_.width, rect_.height,
		SWP_NOACTIVATE | SWP_FRAMECHANGED);

	return 0;
}

void ContainerWindow::OnDpichanged(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	MARGINS margin{ 0, 0, ncm_->captionh, 0 };
	HR(DwmExtendFrameIntoClientArea(hwnd_, &margin));

	cxmin_ = Valfordpi(containerMinWidth_);
	cymin_ = ncm_->captionh + ncm_->hframe;

	assert(lParam);
	RECT* nrc = (RECT*)lParam;

	SetWindowPos(0, nrc->left, nrc->top, nrc->right - nrc->left, nrc->bottom - nrc->top,
		SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOREDRAW1);
}


LRESULT ContainerWindow::OnSize(WPARAM state, int width, int height)
{
	ctab_->Resize();
	//xwidth_ = width;

	RedrawWindow(hwnd_, 0, 0, RDW_VALIDATE | RDW_NOCHILDREN);
	return 0;
}

LRESULT ContainerWindow::OnExitSizemove()
{
	ctab_->Resize();
	return 0;
}

LRESULT ContainerWindow::OnPaint()
{
	return DefWindowProcW(hwnd_, WM_PAINT, 0, 0);
}

LRESULT ContainerWindow::OnMove(int x, int y)
{
	return 0;
}

LRESULT ContainerWindow::OnNcHitTest(LPARAM lparam)
{
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
#if 0
	case WM_SYSCOMMAND:
	{
		if ((wParam & SC_MOVE) == SC_MOVE)
		{
			int i = 0;
			i++;
		}
		break;
	}
	case WM_NCPAINT:
	{
		return 0;
	}
	case WM_WINDOWPOSCHANGING:
	{
		auto pos = (WINDOWPOS*)lParam;
		pos->flags |= SWP_NOMOVE;
		return 0;
	}
#endif	
	case WM_NCCALCSIZE:
	{
		auto& rectw = ((NCCALCSIZE_PARAMS*)lParam)->rgrc[0];
		auto pos = ((NCCALCSIZE_PARAMS*)lParam)->lppos;
#if 1
		if (wParam)
		{
			rectw.right -= ncm_->vframe;
			rectw.left += ncm_->vframe;
			rectw.bottom -= ncm_->hframe;
		}
		return 0;
#else
		if (wParam)
		{
			rectw.top -= ncm_->captionh;
			rectw.top += ncm_->hborder;
		}
		break;
#endif
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

