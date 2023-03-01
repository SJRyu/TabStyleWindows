#pragma once

#include <NativeWindows2/windows/TabWindow.h>

namespace NativeWindows
{
	class NvrTab;

	class TestTab : public TabWindow
	{
	public:

		TestTab(Ctab* parent);
		virtual ~TestTab();

		NvrTab* ttab_ = nullptr;
		NvrTab* ttab1_ = nullptr;

	protected:

		HMENU cmenu_ = nullptr;

		virtual BOOL CALLBACK OnContextMenu(HWND hwnd, int xpos, int ypos) override;
		virtual LRESULT CALLBACK OnCommand(WPARAM wp, LPARAM lp) override;
		virtual LRESULT CALLBACK UserMsgHandler(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) override;

	};
}