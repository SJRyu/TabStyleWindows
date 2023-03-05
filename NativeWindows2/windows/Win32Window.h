#pragma once

#include <winrt/Windows.Foundation.Collections.h>
#include <NativeWindows2/WinUIimports.h>
#include <NativeWindows2/directx/DXimports.h>
#include <NativeWindows2/windows/WindowProperties.h>
#include <NativeWindows2/cctrl/CDialog.h>

namespace NativeWindows
{
	class Win32Window;
	class Win32UIThread;

	struct WinArgs
	{
		RECT* rect = nullptr;
		HMENU hmenu = nullptr;
		Win32Window* parent = nullptr;
		Win32UIThread* thread = nullptr;
	};

	struct NCMeasurement
	{
		UINT dpix = 96;
		UINT dpiy = 96;

		LONG hborder;
		LONG vborder;
		LONG padded;
		LONG hframe;
		LONG vframe;
		LONG captionh;
		LONG sysbtnw;
		LONG sysbtnh;
		LONG sysmenuw;

		LONG tabh;
		LONG taby;

		float d1e0 = 1.0f;
		float d2e0 = 2.0f;
	};

	class NATIVEWINDOWS2_API Win32Window
	{
	public:
		Win32Window* parent_;
		HMENU hmenu_;
		CRECT<LONG> rect_;
		CRECT<LONG> xrect_;

		const WCHAR* wname_ = nullptr;

		Win32UIThread* thread_ = nullptr;
		HWND hwnd_ = nullptr;

		bool isRoot_;
		Win32Window* root_ = nullptr;
		NCMeasurement* ncm_ = nullptr;

		// for owner-tab
		//Win32Window* owner_ = nullptr;

		bool isSizing_ = false;
		bool bScroll_ = false;

		DWORD wstylex_ = 0;
		DWORD wstyle_ = 0;

		HANDLE evclosed_;

		Win32Window(WinArgs const& args);
		virtual ~Win32Window();
		Win32Window() = delete;
		Win32Window(Win32Window const& r) = delete;
		Win32Window& operator=(const Win32Window& r) = delete;

		//Child into child only allowed. and need samethread on parents.
		virtual HWND SetParent(Win32Window* parent);

		// assert for debugging, have to clear in release.
		inline virtual void WINAPI MoveWindow(int x, int y)
		{
			assert(::SetWindowPos(hwnd_, 0, x, y, 0, 0,
				SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOREDRAW | SWP_NOSENDCHANGING) == TRUE);
		}

		inline virtual void WINAPI MoveWindowAsync(int x, int y)
		{
			assert(::SetWindowPos(hwnd_, 0, x, y, 0, 0,
				SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOREDRAW | SWP_NOSENDCHANGING | SWP_ASYNCWINDOWPOS) == TRUE);
		}

		inline virtual void WINAPI SetWindowPos(
			HWND insertafter, int x, int y, int cx, int cy, UINT flags)
		{
			assert(::SetWindowPos(hwnd_, insertafter, x, y, cx, cy, flags) == TRUE);
		}

		inline virtual void WINAPI SetWindowPosAsync(
			HWND insertafter, int x, int y, int cx, int cy, UINT flags)
		{
			flags |= SWP_ASYNCWINDOWPOS;
			assert(::SetWindowPos(hwnd_, insertafter, x, y, cx, cy, flags) == TRUE);
		}

		inline virtual BOOL WINAPI ShowWindow(int nCmdShow = SW_SHOW)
		{
			return ::ShowWindow(hwnd_, nCmdShow);
		}

		inline virtual BOOL WINAPI ShowWindowAsync(int nCmdShow = SW_SHOW)
		{
			return ::ShowWindowAsync(hwnd_, nCmdShow);
		}

		static inline WNDCLASSEX msgclass_;
		static inline WNDCLASSEX windowclass_;
		static inline void RegisterWindowClass()
		{
			msgclass_.cbSize = sizeof(WNDCLASSEX);
			msgclass_.hInstance = g_hinst;
			msgclass_.lpfnWndProc = ThreadMsgProc;
			msgclass_.lpszClassName = L"msgonly";
			ATOM ret = RegisterClassEx(&msgclass_);
			assert(ret != NULL);

			windowclass_.cbSize = sizeof(WNDCLASSEX);
			windowclass_.hInstance = g_hinst;
			windowclass_.lpfnWndProc = EnterProc_;
			windowclass_.lpszClassName = L"win32window";
			windowclass_.hCursor = LoadCursor(NULL, IDC_ARROW);
			windowclass_.style = CS_DBLCLKS;

			ret = RegisterClassEx(&windowclass_);
			assert(ret != NULL);
		}

		static inline void UnregisterWindowClass()
		{
			UnregisterClass(windowclass_.lpszClassName, g_hinst);
			UnregisterClass(msgclass_.lpszClassName, g_hinst);
		}

		virtual void CreateEx()
		{
#pragma warning(push)
#pragma warning(disable : 4312)

			HWND hwnd = CreateWindowExW(
				wstylex_,
				windowclass_.lpszClassName,
				wname_,
				wstyle_,
				rect_.left, rect_.top,
				rect_.width, rect_.height,
				((parent_ == nullptr) ? nullptr : parent_->hwnd_),
				hmenu_,
				g_hinst,
				this);
#pragma warning (pop)
			assert(hwnd_ == hwnd);
		}

		void CreateEx1();
		void WaitClose();

		template <typename T> auto DpiVal(T val)
		{
			return Dpiadj(val, ncm_->dpiy);
		}

	protected:

		static void WINAPI AdjustNcSize(UINT dpi, NCMeasurement* out);

		WNDPROC oldproc_;
		bool bVbi_ = false;

		inline virtual LRESULT CALLBACK OnCreate(LPCREATESTRUCT createstr) { return 0; }
		inline virtual void CALLBACK OnClose() {}
		inline virtual void CALLBACK OnDestroy() {}
		inline virtual void CALLBACK OnDpichanged(UINT uMsg, WPARAM wParam, LPARAM lParam) {}
		//inline virtual LRESULT CALLBACK OnDpichangedBeforeparent() { return 0; }
		inline virtual LRESULT CALLBACK OnDpichangedAfterparent() { return 0; }

		inline virtual LRESULT OnNcHitTest(LPARAM lParam)
		{
			return DefWindowProc(hwnd_, WM_NCHITTEST, 0, lParam);
		};

		inline virtual LRESULT CALLBACK OnEnterSizemove() { return 0; }
		inline virtual LRESULT CALLBACK OnExitSizemove() { return 0; }

		inline virtual void CALLBACK OnSizing(WPARAM wParam, LPARAM lParam) {}
		inline virtual LRESULT CALLBACK OnMove(int x, int y) { return 0; }
		inline virtual LRESULT CALLBACK OnSize(WPARAM state, int width, int height) { return 0; }
		inline virtual LRESULT CALLBACK OnPaint() 
		{ 
			// WM_PAINT는 일반적인 경우에 사용하지 않는다.
			::ValidateRect(hwnd_, nullptr);
			return 0;
		}

		inline virtual LRESULT CALLBACK OnMousemove(int state, int x, int y) { return 0; }
		inline virtual LRESULT CALLBACK OnMouseleave() { return 0; }
		inline virtual LRESULT CALLBACK OnMouseHover(WPARAM wp, LPARAM lp) { return 0; };
		inline virtual BOOL CALLBACK OnMouseWheel(WPARAM wp, LPARAM lp) { return FALSE; };
		inline virtual LRESULT CALLBACK OnLbtndown(int state, int x, int y) { return 0; }
		inline virtual LRESULT CALLBACK OnLbtndouble(int state, int x, int y) { return 0; }
		inline virtual LRESULT CALLBACK OnLbtnup(int state, int x, int y) { return 0; }
		//inline virtual LRESULT CALLBACK OnRbtndown(int stae, int x, int y) { return 0; }
		inline virtual BOOL CALLBACK OnContextMenu(HWND hwnd, int xpos, int ypos) { return FALSE; }
		inline virtual LRESULT CALLBACK OnCommand(WPARAM wp, LPARAM lp) { return FALSE; }

		inline virtual LRESULT CALLBACK OnCButtonClicked(WPARAM wp, LPARAM lp) { return 0; }
		inline virtual LRESULT CALLBACK OnHScrollCmd(WPARAM wp, LPARAM lp) { return 0; }
		inline virtual LRESULT CALLBACK OnVScrollCmd(WPARAM wp, LPARAM lp) { return 0; }
		inline virtual LRESULT CALLBACK OnVbi(WPARAM wp, LPARAM lp) { return 0; }
		inline virtual void CALLBACK OnClientResize(WPARAM wp, LPARAM lp) 
		{ 
			SetWindowPos(0, 0, 0, (int)wp, (int)lp,
				SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOREDRAW1);
		}

		inline virtual LRESULT CALLBACK UserMsgHandler(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
		{
			return DefWindowProc(hWnd, uMsg, wParam, lParam);
		}

	private:

		com_ptr<IDXGISwapChain4> swapchain_;

#if 0
		static BOOL CALLBACK EnumChildOnDpichanged(HWND hwnd, LPARAM lparam)
		{
			::SendMessage(hwnd, UM_DPICHANGED, 0, 0);
			return TRUE;
		}
#endif
		static BOOL CALLBACK EnumChildOnRootChanged(HWND hwnd, LPARAM lparam)
		{
			::SendMessage(hwnd, UM_ROOT_CHANGED, 0, lparam);
			return TRUE;
		}

		static LRESULT CALLBACK ThreadMsgProc(
			HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
		{
			switch (msg)
			{
			case UM_CREATEEX:
			{
				auto win = (Win32Window*)wp;
				win->CreateEx();
				if (lp) win->ShowWindow();
				return 0;
			}
			case UM_CLOSE:
			{
				auto win = (Win32Window*)wp;
				if (win->hwnd_)
				{
					::SendMessage(win->hwnd_, UM_CLOSE, 0, 0);
					SetEvent(win->evclosed_);
				}
				return 0;
			}
			case UM_CREATEDLG:
			{
				auto dlg = (CDialog*)wp;
				dlg->CreateCDlg();
				if (lp) dlg->ShowWindow();
				return 0;
			}
			case UM_CLOSEDLG:
			{
				auto dlg = (CDialog*)wp;
				if (dlg->hwnd_)
				{
					::SendMessage(dlg->hwnd_, UM_CLOSEDLG, 0, 0);
					SetEvent(dlg->evclosed_);
					//::SendMessage(dlg->caller_->hwnd_, UM_DLGDESTROYED, (WPARAM)dlg, 0);
				}
				return 0;
			}
			default:
				break;
			}
			return DefWindowProc(hwnd, msg, wp, lp);
		}

		static LRESULT CALLBACK EnterProc_(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
		virtual LRESULT CALLBACK OnNcCreate_(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

		static LRESULT CALLBACK PreProc_(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
		static LRESULT CALLBACK PreProcForRoot_(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

		LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
		LRESULT CALLBACK WndProcForRoot(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	};

	struct Deleter
	{
		void operator()(Win32Window* p)
		{
			p->WaitClose();
			::delete p;
		}
	};

	template <typename T>
	using wunique_ptr = std::unique_ptr <T, Deleter>;

	template <class _Ty, class... _Types, std::enable_if_t<!std::is_array_v<_Ty>, int> = 0>
	_NODISCARD_SMART_PTR_ALLOC _CONSTEXPR23 wunique_ptr<_Ty> wmake_unique(_Types&&... _Args) 
	{
		return wunique_ptr<_Ty>(new _Ty(_STD forward<_Types>(_Args)...));
	}

	template <class _Ty, std::enable_if_t<std::is_array_v<_Ty>&& std::extent_v<_Ty> == 0, int> = 0>
	_NODISCARD_SMART_PTR_ALLOC _CONSTEXPR23 wunique_ptr<_Ty> wmake_unique(const size_t _Size) 
	{ 
		using _Elem = remove_extent_t<_Ty>;
		return wunique_ptr<_Ty>(new _Elem[_Size]());
	}

	template <class _Ty, class... _Types, std::enable_if_t<std::extent_v<_Ty> != 0, int> = 0>
	void wmake_unique(_Types&&...) = delete;
}
