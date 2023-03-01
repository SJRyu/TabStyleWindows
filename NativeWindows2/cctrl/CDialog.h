#pragma once

#include <NativeWindows2/windows/WindowProperties.h>
#include <NativeWindows2/Win32UIThread.h>

namespace NativeWindows
{
	class Win32UIThread;
	class Win32Window;

	class NATIVEWINDOWS2_API CDialog
	{
	public:
		CDialog(
			HINSTANCE hinst, LPCWSTR rname, 
			Win32Window* caller, Win32UIThread* thread) :
			hinst_(hinst), rname_(rname), caller_(caller), thread_(thread)
		{
			evclosed_ = CreateEvent(NULL, TRUE, TRUE, NULL);
		}

		virtual ~CDialog() 
		{
			CloseHandle(evclosed_);
		}

		CDialog(CDialog const& r) = delete;
		CDialog(CDialog const&& r) = delete;
		CDialog& operator=(CDialog const& r) = delete;
		CDialog& operator=(CDialog const&& r) = delete;

		HWND hwnd_ = nullptr;
		Win32Window* caller_;
		Win32UIThread* thread_;
		HANDLE evclosed_;

		HINSTANCE hinst_;
		LPCWSTR rname_;
		CRECT<LONG> rect_;

		virtual void CreateCDlg()
		{
			HWND hwnd = CreateDialogParam(hinst_, rname_, nullptr, dlgproc_, (LPARAM)this);
			assert(hwnd == hwnd_);
		}

		virtual BOOL ShowWindow(int cmd = SW_SHOW)
		{
			return ::ShowWindow(hwnd_, cmd);
		}

		void CreateCDlg1();
		void WaitClose();

	protected:

		DLGPROC oldproc_ = nullptr;

		virtual LRESULT CALLBACK OnInitDlg() { return TRUE; }
		virtual LRESULT CALLBACK OnSize(WPARAM state, int width, int height) { return FALSE; }
		virtual LRESULT CALLBACK OnCommand(WPARAM wp, LPARAM lp) { return FALSE; }
		virtual LRESULT CALLBACK OnNotify(WPARAM wp, NMHDR* nmhdr) { return FALSE; }
		virtual LRESULT CALLBACK OnPaint(WPARAM wp, LPARAM lp) { return FALSE; };
		virtual void CALLBACK OnClose() {};
		virtual void CALLBACK OnDestroy() {};

		static LRESULT CALLBACK dlgproc_(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
		{
			switch (msg)
			{
			case WM_INITDIALOG:
			{
				auto dlg = (CDialog*)lp;
				dlg->hwnd_ = hwnd;

				SetWindowLongPtr(hwnd, DWLP_USER, (LONG_PTR)dlg);
				dlg->oldproc_ = (DLGPROC)SetWindowLongPtr(hwnd, DWLP_DLGPROC, (LONG_PTR)dlgproc0);

				auto ret = dlg->OnInitDlg();
				ResetEvent(dlg->evclosed_);
				return ret;
			}
			default:
			{
				break;
			}
			}
			return FALSE;
		}

		static LRESULT CALLBACK dlgproc0(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
		{
			auto dlg = (CDialog*)GetWindowLongPtr(hwnd, DWLP_USER);
			return dlg->dlgproc(hwnd, msg, wp, lp);
		}

		LRESULT CALLBACK dlgproc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp);
			
	};
}
