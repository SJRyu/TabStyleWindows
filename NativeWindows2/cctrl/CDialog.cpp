#include <pch.h>
#include <NativeWindows2/cctrl/CDialog.h>
#include <NativeWindows2/Win32UIThread.h>
#include <NativeWindows2/windows/Win32Window.h>

void CDialog::CreateCDlg1()
{
	assert(::SendMessageTimeout(
		thread_->hmsgw_, UM_CREATEDLG, (WPARAM)this, 0,
		SMTO_NOTIMEOUTIFNOTHUNG, 5000, NULL) != 0);
}

void CDialog::WaitClose()
{
	auto threadid = GetCurrentThreadId();

	if (threadid == thread_->threadid_)
	{
		if (hwnd_)
		{
			::SendMessage(hwnd_, UM_CLOSEDLG, 0, 0);
			SetEvent(evclosed_);
		}
	}
	else
	{
		::PostMessage(thread_->hmsgw_, UM_CLOSEDLG, (WPARAM)this, 0);
		DWORD ret;
		while (true)
		{
			ret = MsgWaitForMultipleObjects(
				1, &evclosed_, FALSE, INFINITE, QS_SENDMESSAGE | QS_POSTMESSAGE);
			if (ret == WAIT_OBJECT_0 || ret == WAIT_ABANDONED_0)
			{
				break;
			}
			else if (ret == WAIT_OBJECT_0 + 1)
			{
				MSG Msg;
				if (PeekMessage(&Msg, nullptr, UM_CLOSE, UM_CLOSEDLG, PM_REMOVE))
				{
					TranslateMessage(&Msg);
					DispatchMessage(&Msg);
				}
			}
			else
			{
				assert(0);
			}
		}
	}
}

LRESULT CDialog::dlgproc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
	switch (msg)
	{
	case WM_SIZE:
	{
		int width = LOWORD(lp);
		int height = HIWORD(wp);

		rect_.width = width;
		rect_.height = height;

		return OnSize(wp, width, height);
	}
	case WM_PAINT:
	{
		return OnPaint(wp, lp);
	}
	case WM_COMMAND:
	{
		return OnCommand(wp, lp);
	}
	case WM_NOTIFY:
	{
		return OnNotify(wp, (NMHDR*)lp);
	}
	case WM_CLOSE:
	{
		::PostMessage(caller_->hwnd_, UM_DLG_XBTN, (WPARAM)this, 0);
		return TRUE;
	}
	case UM_CLOSEDLG:
	{
		OnClose();
		::DestroyWindow(hwnd);
		return TRUE;
	}
	case WM_DESTROY:
	{
		OnDestroy();
		SetWindowLongPtr(hwnd, DWLP_DLGPROC, (LONG_PTR)oldproc_);
		hwnd_ = nullptr;
		return FALSE;
	}
	default:
	{
		break;
	}
	}
	return FALSE;
}
