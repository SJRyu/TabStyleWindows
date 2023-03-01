#pragma once

#include <NativeWindows2/windows/Win32Window.h>

namespace NativeWindows
{
	class Win32UIThread;
	class MainLoop;
	class Ctab;
	
	class NATIVEWINDOWS2_API ContainerWindow : public Win32Window
	{
	public:

		CRECT<LONG> xRect_;

		LONG cxmin_;
		LONG cymin_;

		std::unique_ptr<Ctab> ctab_;

		ContainerWindow(RECT* rect, Win32UIThread* thread, int idx, LONG maxtab = 12);
		virtual ~ContainerWindow();

		int idx_;
		LONG tabmaxn_;

		MainLoop* ml_;
		bool isDragbyTab_ = false;
		bool isFindingDoc_ = false;

	protected:

		virtual LRESULT CALLBACK OnCreate(LPCREATESTRUCT createstr) override;
		virtual void CALLBACK OnDpichanged(UINT uMsg, WPARAM wParam, LPARAM lParam) override;
		virtual LRESULT CALLBACK OnMove(int x, int y) override;
		virtual LRESULT CALLBACK OnSize(WPARAM state, int width, int height) override;
		virtual LRESULT CALLBACK OnExitSizemove() override;

		virtual LRESULT CALLBACK OnNcHitTest(LPARAM lparam) override;
		virtual LRESULT CALLBACK UserMsgHandler(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) override;
		virtual void CALLBACK OnClose() override;

	private:


	};
}