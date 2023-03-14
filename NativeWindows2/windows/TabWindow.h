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
	class ClientWindow;

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
			SWP_NOREDRAW1;

		TabWindow(Ctab* parent);
		virtual ~TabWindow();

		wunique_ptr<ClientWindow> client_;
		virtual void SetClient();

		void ResizeClient();
		void WINAPI SetActivate(hoverstatus stat);
		void Reposition();

		virtual void CreateEx() override;

		std::wstring title_;

		std::function<void(TabWindow*)> OnSetup = [](TabWindow*) {};
		std::function<void(TabWindow*)> OnTabClose = [](TabWindow*) {};

		void StopDragging();
		virtual HWND SetParent(Win32Window* parent) override;

		void SetContent(Win32Window* content);

	protected:

		bool dragging_ = false;

		LONG capturex_;
		LONG capturey_;

		SpriteVisual bgvisual_{ nullptr };
		CompositionDrawingSurface bgsurface_{ nullptr };
		//CompositionRoundedRectangleGeometry rrg_{ nullptr };
		SpriteVisual txtvisual_{ nullptr };
		CompositionDrawingSurface txtsurface_{ nullptr };
		DropShadow shadow_{ nullptr };
		ScalarKeyFrameAnimation hoveranime_{ nullptr };

		com_ptr<IDWriteTextFormat3> textformat_;
		DWRITE_FONT_AXIS_VALUE fontaxis_;

		D2D1_GRADIENT_STOP stgradient_[3];
		UINT32 textrgb_ = 0x000000;

		wunique_ptr<CEllipseButton> btnclose_;

		void WINAPI Redraw(int w, int h);
		void WINAPI RedrawFrame(int w, int h);
		void WINAPI RedrawText(int w, int h);

		virtual LRESULT CALLBACK OnCreate1(LPCREATESTRUCT createstr) override;
		virtual LRESULT CALLBACK OnDpichangedAfterparent() override;

		virtual LRESULT CALLBACK OnMove(int x, int y) override;
		virtual LRESULT CALLBACK OnSize(WPARAM state, int width, int height) override;
		virtual LRESULT CALLBACK OnLbtndown(int state, int x, int y) override;
		virtual LRESULT CALLBACK OnLbtnup(int state, int x, int y) override;
		virtual LRESULT CALLBACK OnMousemove(int state, int x, int y) override;
		virtual LRESULT CALLBACK OnMouseleave() override;
		virtual LRESULT CALLBACK OnCButtonClicked(WPARAM wp, LPARAM lp) override;
		//virtual LRESULT CALLBACK OnNcHitTest(LPARAM lparam) override;

		virtual VOID CALLBACK OnClose1() override;
	};
}