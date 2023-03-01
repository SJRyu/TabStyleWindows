#pragma once

#include <NativeWindows2/windows/TabWindow.h>

namespace NativeWindows
{
	class TestTab;

	class NvrTab : public TabWindow
	{
	public:
		
		NvrTab(Ctab* parent);
		virtual ~NvrTab();

		TestTab* ttab_ = nullptr;

	protected:

		HMENU cmenu_ = nullptr;

		virtual BOOL CALLBACK OnContextMenu(HWND hwnd, int xpos, int ypos) override;
		virtual LRESULT CALLBACK OnCommand(WPARAM wp, LPARAM lp) override;
		virtual LRESULT CALLBACK UserMsgHandler(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) override;

	};
}