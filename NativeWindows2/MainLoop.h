#pragma once

#include <NativeWindows2/Win32UIThread.h>
#include <NativeWindows2/windows/Win32Window.h>

namespace NativeWindows
{
	class ContainerWindow;

	class NATIVEWINDOWS2_API MainLoop : public Win32UIThread
	{
	public:

		MainLoop(MainLoop const& ref) = delete;
		MainLoop(MainLoop const&& ref) = delete;
		MainLoop& operator=(MainLoop const& ref) = delete;
		MainLoop& operator=(MainLoop const&& ref) = delete;

		virtual ~MainLoop();

		virtual void Start() override;
		virtual void Start1() override { Start(); };

		std::list<wunique_ptr<ContainerWindow>> windows_;

		ContainerWindow* NewContainer(RECT& rect);
		ContainerWindow* FindDock(int x, int y);
		void CloseContainer(uintptr_t instance);
		void DockContainer();
		void SplitContainer();

	protected:

		MainLoop();

		virtual void CALLBACK OnAppStart() = 0;
		virtual void CALLBACK OnAppEnd() = 0;
		virtual void CALLBACK OnNewContainer(ContainerWindow* win) = 0;

	};
}
