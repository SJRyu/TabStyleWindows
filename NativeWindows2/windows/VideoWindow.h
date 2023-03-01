#pragma once

#include <NativeWindows2/windows/Win32Window.h>
#include <NativeWindows2/directx/VideoComponents.h>

namespace NativeWindows
{
	class NATIVEWINDOWS2_API VideoWindow : public Win32Window, public VideoComponents
	{
	public:

		VideoWindow(WinArgs const& args);
		virtual ~VideoWindow();

		
	protected:

		virtual LRESULT CALLBACK OnCreate(LPCREATESTRUCT createstr) override;
		virtual LRESULT CALLBACK OnCreate1(LPCREATESTRUCT createstr) { return 0; }
		virtual void CALLBACK OnClose() override;
		virtual void CALLBACK OnClose1() {}
	};
}