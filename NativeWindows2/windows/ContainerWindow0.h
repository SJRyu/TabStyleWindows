#pragma once

#include <NativeWindows2/windows/D2dWindow.h>

namespace NativeWindows
{
	class Win32UIThread;
	class Ctab;
	
	class NATIVEWINDOWS2_API ContainerWindow : public D2dWindow
	{
	public:

		LONG cxmin_;
		LONG cymin_;

		Ctab* ctab_ = nullptr;
		int xwidth_ = 0;

		ContainerWindow(RECT* rect, Win32UIThread* thread);
		virtual ~ContainerWindow();

		void Close();

	protected:

		virtual LRESULT CALLBACK OnCreate1(LPCREATESTRUCT createstr) override;
		virtual void CALLBACK OnDpichanged(UINT uMsg, WPARAM wParam, LPARAM lParam) override;
		virtual LRESULT CALLBACK OnMove(int x, int y) override;
		virtual LRESULT CALLBACK OnSize(WPARAM state, int width, int height) override;
		virtual LRESULT CALLBACK OnPaint() override;
		virtual LRESULT CALLBACK OnExitSizemove() override;

		virtual LRESULT CALLBACK OnNcHitTest(LPARAM lparam) override;
		virtual LRESULT CALLBACK UserMsgHandler(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) override;
		void CALLBACK OnDestroy() override;

	private:


	};
}