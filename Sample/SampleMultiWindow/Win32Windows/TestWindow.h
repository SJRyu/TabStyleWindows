#pragma once

#include <NativeWindows2/windows/D2dWindow.h>

namespace NativeWindows
{
	class TestTab;

	class TestWindow : public D2dWindow
	{
	public:

		TestWindow(TestTab* tab);
		virtual ~TestWindow();

		TestTab* tab_;

	protected:

		virtual LRESULT CALLBACK OnCreate1(LPCREATESTRUCT createstr) override;
		virtual LRESULT CALLBACK OnSize(WPARAM state, int width, int height) override;
		virtual VOID CALLBACK OnClose1() override;

	};
}
