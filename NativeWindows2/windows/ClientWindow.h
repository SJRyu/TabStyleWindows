#pragma once

#include <NativeWindows2/windows/ScrollWindow.h>

namespace NativeWindows
{
	class Win32UIThread;
	class TabWindow;
	class Ctab;
	class ContainerWindow;
	class ClientScroll;

	class NATIVEWINDOWS2_API ClientWindow : public D2dWindow
	{
	public:
		ClientWindow(TabWindow* tab);
		virtual ~ClientWindow();

		std::unique_ptr<Win32UIThread> cthread_;
		TabWindow* tab_;
		ContainerWindow* container_;

		SpriteVisual visualbg_{ nullptr };

		inline void SetContent(Win32Window* win) { content_ = win; };
		inline auto GetContent() { return content_; }
		
		virtual void CreateEx() override;
		virtual HWND SetParent(Win32Window* parent) override;

	protected:

		Win32Window* content_ = nullptr;
		wunique_ptr<ClientScroll> scwin_;

		virtual LRESULT CALLBACK OnCreate1(LPCREATESTRUCT createstr) override;
		virtual LRESULT CALLBACK OnSize(WPARAM state, int width, int height) override;
		virtual void CALLBACK OnClose1() override;

	};

	class NATIVEWINDOWS2_API ClientScroll : public ScrollWindow
	{
	public:
		
		ClientScroll(WinArgs const& args) : ScrollWindow(args) {}
		~ClientScroll() {}

		virtual void SetTarget(Win32Window* target) override
		{
			target->rect_ = rect_;
			ScrollWindow::SetTarget(target);
		}

	protected:

		virtual void CALLBACK OnTargetSize(int width, int height) override
		{
			// do nothing, we don't need to update scroll in this case.
		}
		virtual LRESULT CALLBACK OnSize(WPARAM state, int width, int height) override;
	};
}
