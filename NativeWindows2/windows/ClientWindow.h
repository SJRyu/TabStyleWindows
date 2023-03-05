#pragma once

#include <NativeWindows2/windows/ScrollWindow.h>

namespace NativeWindows
{
	class Win32UIThread;
	class TabWindow;
	class Ctab;
	class ContainerWindow;

	class ClientWindow : public ScrollWindow
	{
	public:
		ClientWindow(TabWindow* tab);
		virtual ~ClientWindow();

		std::unique_ptr<Win32UIThread> thread_;
		wunique_ptr<Win32Window> content_;
		TabWindow* tab_;
		ContainerWindow* container_;
		
		virtual void CreateEx() override;
		virtual HWND SetParent(Win32Window* parent) override;

	protected:

		virtual LRESULT CALLBACK OnCreate1(LPCREATESTRUCT createstr) override;
		virtual LRESULT CALLBACK OnSize1(WPARAM state, int width, int height) override;
		virtual void CALLBACK OnClose1() override;

	};
}
