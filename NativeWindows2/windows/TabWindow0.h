#pragma once

#include <NativeWindows2/Win32UIThread.h>
#include <NativeWindows2/windows/D2dWindow.h>

namespace NativeWindows
{
	class Ctab;
	class ContainerWindow;
	class CEllipseButton;
	class CustomTextRenderer;
	class Win32UIThread;

	using namespace Windows::UI::Composition;
	namespace abicomp = ABI::Windows::UI::Composition;	

	class NATIVEWINDOWS2_API TabWindow : public D2dWindow1
	{
	public:

		Ctab* ctab_;
		ContainerWindow* refcontainer_;

		int position_;
		int status_ = hover_none;

		int reposflagforcontent =
			SWP_NOZORDER | SWP_NOACTIVATE |
			SWP_NOREDRAW1 | SWP_ASYNCWINDOWPOS;

		TabWindow(Ctab* parent);
		virtual ~TabWindow();

		std::unique_ptr<Win32Window> client_;

		void ResizeClient();

		void WINAPI SetActivate(hoverstatus stat);
		void Reposition();

		virtual void OnTabClose() {};
		virtual void CreateEx() override;

	protected:

		bool dragging_ = false;
		LONG capturepoint_;

		SpriteVisual bgvisual_{ nullptr };
		com_ptr<abicomp::ICompositionDrawingSurfaceInterop> bgsurface_;
		CompositionRoundedRectangleGeometry rrg_{ nullptr };
		SpriteVisual txtvisual_{ nullptr };
		com_ptr<abicomp::ICompositionDrawingSurfaceInterop> txtsurface_;
		DropShadow shadow_{ nullptr };
		ScalarKeyFrameAnimation hoveranime_{ nullptr };

		std::wstring title_;
		com_ptr<IDWriteTextFormat3> textformat_;
		DWRITE_FONT_AXIS_VALUE fontaxis_;

		D2D1_GRADIENT_STOP stgradient_[3];
		UINT32 textrgb_ = 0x000000;

		CEllipseButton* btnclose_ = nullptr;

		void Redraw();

		virtual LRESULT CALLBACK OnCreate1(LPCREATESTRUCT createstr) override;
		virtual void OnSetup() { }
		virtual LRESULT CALLBACK OnDpichangedAfterparent() override;

		virtual LRESULT CALLBACK OnMove(int x, int y) override;
		virtual LRESULT CALLBACK OnSize(WPARAM state, int width, int height) override;
		virtual LRESULT CALLBACK OnPaint() override;
		virtual LRESULT CALLBACK OnLbtndown(int state, int x, int y) override;
		virtual LRESULT CALLBACK OnLbtnup(int state, int x, int y) override;
		virtual LRESULT CALLBACK OnMousemove(int state, int x, int y) override;
		virtual LRESULT CALLBACK OnMouseleave() override;
	};
}