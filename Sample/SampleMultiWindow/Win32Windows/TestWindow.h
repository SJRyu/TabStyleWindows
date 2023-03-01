#pragma once

#include <NativeWindows2/windows/Win32Window.h>

namespace NativeWindows
{
	class TestTab;

	class TestWindow : public Win32Window
	{
	public:

		TestWindow(TestTab* tab);
		virtual ~TestWindow();

		TestTab* tab_;

	protected:

		virtual LRESULT CALLBACK OnCreate(LPCREATESTRUCT createstr) override;
		virtual LRESULT CALLBACK OnSize(WPARAM state, int width, int height) override;
		virtual VOID CALLBACK OnClose() override;

	};
}
