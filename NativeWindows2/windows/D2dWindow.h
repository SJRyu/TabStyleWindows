#pragma once

#include <NativeWindows2/windows/Win32Window.h>
#include <NativeWindows2/directx/D2dComponents.h>
#include <NativeWindows2/windows/WindowProperties.h>

namespace NativeWindows
{
	class Win32UIThread;

	class NATIVEWINDOWS2_API D2dWindow : public Win32Window, public D2dComponents
	{
	public:
		inline D2dWindow(WinArgs const& args) :
			Win32Window(args), D2dComponents(this) {}

		virtual ~D2dWindow() 
		{
		}

	protected:

		virtual LRESULT CALLBACK OnCreate1(LPCREATESTRUCT createstr) { return 0; };
		virtual LRESULT CALLBACK OnCreate(LPCREATESTRUCT createstr) override
		{
			InitCompositor();
			return OnCreate1(createstr);
		}

		virtual void CALLBACK OnClose1() {};
		virtual void CALLBACK OnClose() override
		{
			OnClose1();
#ifdef WINRT_CLOSETEST
			ReleaseCompositor();
#endif
		}
	};

	struct D2dWinArgs
	{
		RECT* rect = nullptr;
		HMENU hmenu = nullptr;
		D2dWindow* parent = nullptr;
		Win32UIThread* thread = nullptr;
	};

	class NATIVEWINDOWS2_API D2dWindow1 : public D2dWindow
	{
	public:
		inline D2dWindow1(D2dWinArgs const& args) :
			D2dWindow({ args.rect, args.hmenu, args.parent, args.thread }),
			d2dparent_(args.parent) {}

		virtual ~D2dWindow1()
		{
			if (winrt::get_abi(parentv_))
			{
				parentv_.Remove(rootv_);
			}
		}

		D2dWindow* d2dparent_;

		virtual BOOL WINAPI ShowWindow(int nCmdShow = SW_SHOW) override
		{
			if (nCmdShow == SW_HIDE)
			{
				rootv_.IsVisible(false);
			}
			else
			{
				rootv_.IsVisible(true);
			}
			return ::ShowWindow(hwnd_, nCmdShow);
		}

		virtual BOOL WINAPI ShowWindowAsync(int nCmdShow = SW_SHOW) override
		{
			if (nCmdShow == SW_HIDE)
			{
				rootv_.IsVisible(false);
			}
			else
			{
				rootv_.IsVisible(true);
			}
			return ::ShowWindowAsync(hwnd_, nCmdShow);
		}

		virtual void WINAPI MoveWindow(int x, int y) override
		{
			MoveVisuals(x, y);
			Win32Window::MoveWindow(x, y);
		}

		virtual void WINAPI MoveWindowAsync(int x, int y) override
		{
			MoveVisuals(x, y);
			Win32Window::MoveWindowAsync(x, y);
		}

		virtual void WINAPI SetWindowPos(
			HWND insertafter, int x, int y, int cx, int cy, UINT flags) override
		{
			if (!(flags & SWP_NOMOVE)) MoveVisuals(x, y);
			if (!(flags & SWP_NOSIZE)) SizeVisuals(cx, cy);
			Win32Window::SetWindowPos(insertafter, x, y, cx, cy, flags);
		}

		virtual void WINAPI SetWindowPosAsync(
			HWND insertafter, int x, int y, int cx, int cy, UINT flags) override
		{
			if (!(flags & SWP_NOMOVE)) MoveVisuals(x, y);
			if (!(flags & SWP_NOSIZE)) SizeVisuals(cx, cy);
			Win32Window::SetWindowPosAsync(insertafter, x, y, cx, cy, flags);
		}

		virtual void WINAPI AnimeMoveWindow(int x, int y)
		{
			AnimeMoveVisuals(x, y);
			Win32Window::MoveWindow(x, y);
		}

		virtual void WINAPI AnimeMoveWindowAsync(int x, int y)
		{
			AnimeMoveVisuals(x, y);
			Win32Window::MoveWindowAsync(x, y);
		}

		virtual void WINAPI AnimeWindowPos(
			HWND insertafter, int x, int y, int cx, int cy, UINT flags)
		{
			if (!(flags & SWP_NOMOVE)) AnimeMoveVisuals(x, y);
			if (!(flags & SWP_NOSIZE)) AnimeSizeVisuals(cx, cy);
			Win32Window::SetWindowPos(insertafter, x, y, cx, cy, flags);
		}

		virtual void WINAPI AnimeWindowPosAsync(
			HWND insertafter, int x, int y, int cx, int cy, UINT flags)
		{
			if (!(flags & SWP_NOMOVE)) AnimeMoveVisuals(x, y);
			if (!(flags & SWP_NOSIZE)) AnimeSizeVisuals(cx, cy);
			Win32Window::SetWindowPosAsync(insertafter, x, y, cx, cy, flags);
		}

	protected:

		virtual LRESULT CALLBACK OnCreate(LPCREATESTRUCT createstr) override
		{
			InitCompositor1(d2dparent_);
			if (wstyle_ & WS_VISIBLE)
			{
				rootv_.IsVisible(true);
			}
			else
			{
				rootv_.IsVisible(false);
			}
			return OnCreate1(createstr);
		}

		virtual void CALLBACK OnClose() override
		{
			OnClose1();
#ifdef WINRT_CLOSETEST
			ReleaseCompositor1();
#endif
		}
	};

#if 0
template <typename... Args>
inline D2dWindow(D2D1_COLOR_F&& bg, Args&&... args)
	: Win32Window(std::forward<Args>(args)...),
	D2dComponents(this), bgColor_(bg) {}

template <typename... Args>
inline D2dWindow(Args&&... args)
	: Win32Window(std::forward<Args>(args)...),
	D2dComponents(this), bgColor_({ 0.f, 0.f, 0.f, 0.f }) {}
#endif
}
