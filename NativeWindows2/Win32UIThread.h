#pragma once

#include <winrt/Windows.Foundation.Collections.h>
#include <NativeWindows2/windows/WindowProperties.h>
#include <NativeWindows2/directx/IndependentRes.h>

namespace NativeWindows
{
	class Win32Window;

	class NATIVEWINDOWS2_API Win32UIThread
	{
	public:

		HANDLE hthread_ = nullptr;
		DWORD threadid_ = 0;

		HANDLE evStarted_ = nullptr;

		HWND hmsgw_ = nullptr;

		Win32UIThread();
		virtual ~Win32UIThread();
		Win32UIThread(const Win32UIThread& r) = delete;
		Win32UIThread& operator=(const Win32UIThread& r) = delete;

		virtual void Start();
		virtual void Start1();
		virtual void Close();

		std::function<void(Win32UIThread*)> OnThreadStarts = [](Win32UIThread*) {};
		std::function<void(Win32UIThread*)> OnThreadEnds = [](Win32UIThread*) {};

		inline auto Res()
		{
			return res_.get();
		}

	protected:

		//winrt::fire_and_forget DispatchAsync(winrt::Windows::System::DispatcherQueue queue);
		winrt::Windows::Foundation::IAsyncAction DispatchAsync(winrt::Windows::System::DispatcherQueue queue);

		std::unique_ptr<IndependentRes> res_;

		static DWORD WINAPI threadroutine(void* instance);
		virtual DWORD threadroutine();
		virtual void handlemsg()
		{
			MSG Msg;
			while (GetMessage(&Msg, nullptr, 0, 0))
			{
				switch (Msg.message)
				{
				case UTM_QUIT:
				{
					::DestroyWindow(hmsgw_);
					::PostQuitMessage(0);
					break;
				}
				default:
				{
					TranslateMessage(&Msg);
					DispatchMessage(&Msg);
					break;
				}
				}
			}
		}

	private:

	};
}