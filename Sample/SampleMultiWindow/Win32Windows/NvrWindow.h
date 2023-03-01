#pragma once

#include <NativeWindows2/windows/Win32Window.h>

namespace NativeWindows
{
	class NvrTab;
	class VideoView;

	// if you want background, inherit D2dWindow instead.
	class NvrWindow : public Win32Window
	{
	public:

		NvrWindow(NvrTab* tab);
		virtual ~NvrWindow();

		NvrTab* tab_;
		std::array<wunique_ptr<VideoView>, MAX_CAM_COUNT> views_;

		virtual void CreateEx() override
		{
			rect_.width = std::max((int)rect_.width, DpiVal(MIN_WIDTH));
			rect_.height = std::max((int)rect_.height, DpiVal(MIN_HEIGHT));
			Win32Window::CreateEx();
		}

		virtual void WINAPI SetWindowPos(
			HWND insertafter, int x, int y, int cx, int cy, UINT flags) override
		{
			if ((flags & SWP_NOSIZE) == 0)
			{
				cx = std::max(cx, DpiVal(MIN_WIDTH));
				cy = std::max(cy, DpiVal(MIN_HEIGHT));
			}
			assert(::SetWindowPos(hwnd_, insertafter, x, y, cx, cy, flags) == TRUE);
		}

		virtual void WINAPI SetWindowPosAsync(
			HWND insertafter, int x, int y, int cx, int cy, UINT flags) override
		{
			if ((flags & SWP_NOSIZE) == 0)
			{
				auto w = DpiVal(MIN_WIDTH);
				auto h = DpiVal(MIN_HEIGHT);
				cx = std::max(cx, w);
				cy = std::max(cy, h);
			}
			flags |= SWP_ASYNCWINDOWPOS;
			assert(::SetWindowPos(hwnd_, insertafter, x, y, cx, cy, flags) == TRUE);
		}

	protected:

		static constexpr int MIN_WIDTH = 960;
		static constexpr int MIN_HEIGHT = 640;

		void WINAPI CalcChildRect(int pos, CRECT<LONG>* outref);

		virtual LRESULT CALLBACK OnCreate(LPCREATESTRUCT createstr) override;
		virtual LRESULT CALLBACK OnSize(WPARAM state, int width, int height) override;
		virtual VOID CALLBACK OnClose() override;

	};
}
