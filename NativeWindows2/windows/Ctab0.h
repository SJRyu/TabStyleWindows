#pragma once

#include <NativeWindows2/windows/D2dWindow.h>

namespace NativeWindows
{
	class ContainerWindow;
	class TabWindow;
	class Win32UIThread;
	class CEllipseButton;

	class NATIVEWINDOWS2_API Ctab : public D2dWindow1
	{
	public:
		Ctab(ContainerWindow* container);
		virtual ~Ctab();

		ContainerWindow* refcontainer_ = nullptr;

		const LONG tabmaxn_ = 16;
		LONG tabmaxw_;
		FLOAT tabwidth_;
		LONG tabend_;

		LONG btnaddx_;
		LONG btnaddy_;
		LONG btnaddr_;
		LONG btnaddend_;

		std::unique_ptr<CEllipseButton> btnadd_;

		std::list<std::unique_ptr<TabWindow>> tabs_;
		int current_ = -1;

		void WINAPI CalcTabwidth(int count);
		inline void CalcTabwidth(size_t count)
		{
			CalcTabwidth((int)count);
		}

		void Resize();
		void ReposAlltabs();
		void AddTab(TabWindow* tab);
		void WINAPI RemoveTab(int idx);
		void WINAPI ActiveTab(int idx);
		void WINAPI SwapTab(int active, int inactive);

	protected:

		virtual LRESULT CALLBACK OnCreate1(LPCREATESTRUCT createstr) override;
		virtual LRESULT CALLBACK OnDpichangedAfterparent() override;
		virtual LRESULT CALLBACK OnSize(WPARAM state, int width, int height) override;
		virtual LRESULT CALLBACK OnMove(int x, int y) override;
		virtual LRESULT CALLBACK OnCButtonClicked(WPARAM wp, LPARAM lp) override;
		//virtual LRESULT CALLBACK UserMsgHandler(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) override;

		//virtual LRESULT CALLBACK OnPaint() override;
		//void CALLBACK OnDestroy() override;

	private:
	};
}