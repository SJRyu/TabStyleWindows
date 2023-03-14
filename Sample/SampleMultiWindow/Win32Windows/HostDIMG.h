#pragma once

#include <NativeWindows2/Win32UIThread.h>
#include <NativeWindows2/windows/D2dWindow.h>

namespace NativeWindows
{
	class HostDIMG : public Win32UIThread, public D2dWindow
	{
	public:

		HostDIMG();
		virtual ~HostDIMG();

		virtual void CreateEx1() override
		{
			Start1();
		}

	protected:

		bool bDone_ = false;
		bool bShow_ = false;

		SpriteVisual visualf_{ nullptr };
		ICompositionSurface surfacef_{ nullptr };

		//com_ptr<ID3D11Texture2D> backbuffer_;
		com_ptr<IDXGISwapChain1> swapchain_;
		ID3D11RenderTargetView* renderTargetView_;

		virtual void handlemsg() override;
		void handlemsg_(MSG Msg);

		void CreateSwapchain();
		void ResizeSwapchain(int w, int h);

		virtual LRESULT CALLBACK OnCreate1(LPCREATESTRUCT createstr) override;
		virtual LRESULT CALLBACK OnSize(WPARAM state, int width, int height) override;
		virtual LRESULT WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) override;
	};
}
