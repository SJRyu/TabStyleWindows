#include <pch.h>
#include <Win32Windows/TestTab.h>
#include <NativeWindows2/windows/ClientWindow.h>
#include <NativeWindows2/windows/Ctab.h>
#include <NativeWindows2/windows/ContainerWindow.h>
#include <Win32Windows/TestWindow.h>
#include <Win32Windows/NvrTab.h>

using namespace NativeWindows;

TestTab::TestTab(Ctab* parent) : TabWindow(parent)
{
	title_ = L"Viewer ##";

	OnSetup = [](TabWindow* sender)
	{
		TestTab* tab = (TestTab*)sender;
		auto content = new TestWindow(tab);
		tab->SetContent(content);

		SetTimer(tab->hwnd_, 1, 200, NULL);
	};

	OnTabClose = [](TabWindow* sender)
	{
		KillTimer(sender->hwnd_, 1);
	};
}

TestTab::~TestTab()
{
	
}

BOOL TestTab::OnContextMenu(HWND hwnd, int xpos, int ypos)
{

	return TRUE;
}

LRESULT TestTab::OnCommand(WPARAM wp, LPARAM lp)
{


	return FALSE;
}

LRESULT TestTab::UserMsgHandler(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_TIMER:
	{
		if (wParam == 1)
		{
			if (ttab_ == nullptr)
			{
				ttab_ = new NvrTab(ctab_);
				ctab_->PostAdd(ttab_);
			}
			else
			{
				ctab_->PostRemove(ttab_);
				ttab_ = nullptr;
			}

			if (ttab1_ == nullptr)
			{
				ttab1_ = new NvrTab(ctab_);
				ctab_->PostAdd(ttab1_);
			}
			else
			{
				ctab_->PostRemove(ttab1_);
				ttab1_ = nullptr;
			}

			return 0;
		}
		else
		{
			break;
		}
	}
	default:
	{
		break;
	}
	}
	return ::DefWindowProc(hWnd, uMsg, wParam, lParam);
}