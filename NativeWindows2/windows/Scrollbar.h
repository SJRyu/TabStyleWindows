#pragma once

#include <NativeWindows2/windows/D2dWindow.h>

namespace NativeWindows
{
	// I dont want any send/post messages for these.
	class ScrollWindow;
	class Scrollbar;

	class NATIVEWINDOWS2_API Scrollball : public D2dWindow1
	{
	public:

		Scrollball(D2dWinArgs const& args);
		virtual ~Scrollball();

	protected:

		bool bHorizontal_ = false;

		Scrollbar* bar_;
		SpriteVisual visualb_{ nullptr };

		bool bDrag_ = false;
		int capturex_ = 0;
		int capturey_ = 0;

		virtual LRESULT CALLBACK OnCreate1(LPCREATESTRUCT createstr) override;
		virtual void CALLBACK OnClose1() override;
		virtual LRESULT CALLBACK OnMove(int x, int y) override;
		virtual LRESULT CALLBACK OnSize(WPARAM state, int width, int height) override;
		virtual LRESULT CALLBACK OnLbtndown(int state, int x, int y) override;
		virtual LRESULT CALLBACK OnLbtnup(int state, int x, int y) override;
		virtual LRESULT CALLBACK OnMousemove(int state, int x, int y) override;
	};

	struct ScrollInfo
	{
		int clientSize_;
		int pageSize_;
		int clientLoc_;
	};

	class NATIVEWINDOWS2_API Scrollbar : public ScrollInfo, public D2dWindow
	{
	public:

		Scrollbar(WinArgs const& args, bool bHorizontal = false);
		virtual ~Scrollbar();

		Scrollbar& operator=(ScrollInfo const& r)
		{
			*((ScrollInfo*)this) = r;
			return *this;
		}

		ScrollWindow* scrollwin_;
		SpriteVisual visualb_{ nullptr };

		wunique_ptr<Scrollball> ball_;

		void UpdateScrollbar();
		void UpdateScrollbar(ScrollInfo const& info)
		{
			*this = info;
			UpdateScrollbar();
		}

		void MoveBall(int x, int y);
		inline bool IsHorizontal() { return bHorizontal_; }

	protected:

		bool bHorizontal_;

		virtual LRESULT CALLBACK OnCreate1(LPCREATESTRUCT createstr) override;
		virtual void CALLBACK OnClose1() override;
		virtual LRESULT CALLBACK OnSize(WPARAM state, int width, int height) override;
		virtual LRESULT CALLBACK OnLbtndown(int state, int x, int y) override;
		virtual LRESULT CALLBACK OnLbtnup(int state, int x, int y) override;
		virtual LRESULT CALLBACK UserMsgHandler(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) override;

	};
}
