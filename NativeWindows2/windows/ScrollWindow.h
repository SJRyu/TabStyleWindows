#pragma once

#include <NativeWindows2/windows/D2dWindow.h>

namespace NativeWindows
{
	class MainLoop;
	// no smooth scroll for now
	class NATIVEWINDOWS2_API ScrollWindow : public Win32Window
	{
	public:

		ScrollWindow(WinArgs const& args);
		virtual ~ScrollWindow();

		Win32Window* target_ = nullptr;
		MainLoop* ml_;

		inline void SetTarget(Win32Window* target) 
		{ 
			target_ = target;
			target->bScroll_ = true;
			UpdateScroll();
		}

		bool VScrollup();
		bool VScrolldown();
		bool HScrollup();
		bool HScrolldown();

		//update scrollbar
		void UpdateScroll();

	protected:

		virtual LRESULT CALLBACK OnCreate(LPCREATESTRUCT createstr) override;
		virtual LRESULT CALLBACK OnCreate1(LPCREATESTRUCT createstr) { return 0; };
		virtual void CALLBACK OnClose() override;
		virtual void CALLBACK OnClose1() {};
		virtual LRESULT CALLBACK OnSize(WPARAM state, int width, int height) override;
		virtual LRESULT CALLBACK OnSize1(WPARAM state, int width, int height) { return 0; }
		virtual BOOL CALLBACK OnMouseWheel(WPARAM wp, LPARAM lp) override;
		virtual LRESULT CALLBACK UserMsgHandler(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) override;
	};

}
