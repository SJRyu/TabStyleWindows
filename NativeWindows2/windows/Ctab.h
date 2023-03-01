#pragma once

#include <NativeWindows2/windows/D2dWindow.h>
#include <NativeWindows2/Win32UIThread.h>

namespace NativeWindows
{
	class ContainerWindow;
	class TabWindow;
	class CEllipseButton;

	class NATIVEWINDOWS2_API Ctab : public D2dWindow
	{
	public:
		Ctab(ContainerWindow* container);
		virtual ~Ctab();

		ContainerWindow* refcontainer_ = nullptr;

		LONG tabmaxn_;
		LONG tabmaxw_;
		FLOAT tabwidth_;
		LONG tabend_;

		LONG btnaddx_;
		LONG btnaddy_;
		LONG btnaddr_;
		LONG btnaddend_;

		wunique_ptr<CEllipseButton> btnadd_;

		std::list<wunique_ptr<TabWindow>> tabs_;
		TabWindow* tab_ = nullptr;
		int current_ = -1;

		void WINAPI CalcTabwidth(int count);
		inline void CalcTabwidth(size_t count)
		{
			CalcTabwidth((int)count);
		}

		void Resize(BOOL resizing = FALSE);
		void ReposAlltabs();

		inline void PostResize(BOOL resizing = FALSE)
		{
			::PostMessage(hwnd_, UM_TAB_RESIZE, resizing, 0);
		}

		inline void PostAdd(TabWindow* tab)
		{
			::PostMessage(hwnd_, UM_TAB_ADD, (WPARAM)tab, 0);
		}

		inline void PostRemove(TabWindow* tab)
		{
			::PostMessage(hwnd_, UM_TAB_REMOVE, (WPARAM)tab, 0);
		}

		inline void PostActivate(int idx)
		{
			::PostMessage(hwnd_, UM_TAB_ACTIVATE, (WPARAM)idx, 0);
		}

		inline void PostActivate(TabWindow* tab)
		{
			::PostMessage(hwnd_, UM_TAB_ACTIVATE1, (WPARAM)tab, 0);
		}

		virtual void CreateEx() override;
		std::function<void(Ctab*)> OnSetup = [](Ctab*) {};

		bool WINAPI AddTab(TabWindow* tab);
		bool WINAPI RemoveTab(int idx);
		bool WINAPI ActiveTab(int idx);
		bool WINAPI SwapTab(int active, int inactive);
		bool WINAPI RemoveTab(TabWindow* tab);
		bool WINAPI ActiveTab(TabWindow* tab);

		bool WINAPI InsertTab(TabWindow* tab);
		bool WINAPI MoveoutTab(TabWindow* tab);

	protected:

		virtual LRESULT CALLBACK OnCreate1(LPCREATESTRUCT createstr) override;
		virtual LRESULT CALLBACK OnDpichangedAfterparent() override;
		virtual LRESULT CALLBACK OnSize(WPARAM state, int width, int height) override;
		virtual LRESULT CALLBACK OnMove(int x, int y) override;
		virtual LRESULT CALLBACK OnCButtonClicked(WPARAM wp, LPARAM lp) override;
		virtual LRESULT CALLBACK OnNcHitTest(LPARAM lparam) override;
		virtual LRESULT CALLBACK UserMsgHandler(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) override;
		virtual VOID CALLBACK OnClose1() override;

	private:
	};
}