#pragma once

#include <NativeWindows2/windows/D2dWindow.h>

/**
* There is a miss location of client, when DPI changed.
**/
namespace NativeWindows
{
	class MainLoop;
	class Scrollbar;
	
	class NATIVEWINDOWS2_API ScrollNotch : public D2dWindow
	{
	public:

		ScrollNotch(WinArgs const& args);
		virtual ~ScrollNotch();

	protected:

		SpriteVisual visualb_{ nullptr };

		virtual LRESULT CALLBACK OnCreate1(LPCREATESTRUCT createstr) override;
		virtual void CALLBACK OnClose1() override;
		virtual LRESULT CALLBACK OnLbtndown(int state, int x, int y) override;
		virtual LRESULT CALLBACK UserMsgHandler(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) override;
	};

	class NATIVEWINDOWS2_API ScrollWindow : public D2dWindow
	{
	public:

		ScrollWindow(WinArgs const& args);
		virtual ~ScrollWindow();

		wunique_ptr<Win32Window> target_;
		MainLoop* ml_;

		wunique_ptr<ScrollNotch> notch_;
		wunique_ptr<Scrollbar> vscroll_;
		wunique_ptr<Scrollbar> hscroll_;

		virtual void SetTarget(Win32Window* target)
		{
			target->bScroll_ = true;
			target->SetWindowArgs({ 0, 0, this, thread_ });
			target_ = wunique_ptr<Win32Window>(target);
		}

		bool VScrollup();
		bool VScrolldown();
		bool HScrollup();
		bool HScrolldown();

		//update scrollbar
		void UpdateScroll();
		void UpdateScrollbar();

		void OnScrollChanged(int newcl, bool bHorizontal);

	protected:

		CRECT<LONG> targetRect_;
		CRECT<LONG> clientRect_;

		bool isVScrolling_ = false;
		bool isHScrolling_ = false;

		virtual void ScrollTarget(int x, int y);

		virtual void CALLBACK OnTargetSize(int width, int height)
		{
			// in some case, we don't need to update scroll.
			// for an example, this scroll resize target and update already. 
			UpdateScroll();
			UpdateScrollbar();
		}
		virtual void CALLBACK OnTargetMove(int x, int y) 
		{
			// This event would not be fired by scroll, 
			// then we should update scroll.
			// UpdateScroll();
			UpdateScrollbar();
		}

		virtual LRESULT CALLBACK OnCreate(LPCREATESTRUCT createstr) override;
		virtual LRESULT CALLBACK OnCreate1(LPCREATESTRUCT createstr) { return 0; };
		virtual void CALLBACK OnClose() override;
		virtual void CALLBACK OnClose1() {};
		virtual LRESULT CALLBACK OnSize(WPARAM state, int width, int height) override;
		virtual LRESULT CALLBACK OnSize1(WPARAM state, int width, int height) { return 0; }
		virtual LRESULT CALLBACK OnDpichangedAfterparent() override;
		virtual BOOL CALLBACK OnMouseWheel(WPARAM wp, LPARAM lp) override;
		virtual LRESULT CALLBACK UserMsgHandler(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) override;
	};
}
