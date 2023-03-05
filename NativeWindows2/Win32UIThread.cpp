#include <pch.h>
#include <NativeWindows2/Win32UIThread.h>
#include <NativeWindows2/windows/Win32Window.h>

Win32UIThread::Win32UIThread()
{
	res_ = std::unique_ptr<IndependentRes>(new IndependentRes());

	evStarted_ = CreateEventW(
		nullptr,
		true, // manual reset
		false, // initial set
		nullptr
	);
	assert(evStarted_ != nullptr);
}

Win32UIThread::~Win32UIThread()
{
	Close();
	SafeClose(&evStarted_);
}

void Win32UIThread::Start()
{
	hthread_ = CreateThread(
		nullptr,
		0,
		threadroutine,
		this,
		0,
		&threadid_
	);
	//assert(WaitForSingleObject(evStarted_, 5000) == WAIT_OBJECT_0);
	assert(WaitForSingleObject(evStarted_, INFINITE) == WAIT_OBJECT_0);
}

void Win32UIThread::Start1()
{
	hthread_ = CreateThread(
		nullptr,
		0,
		threadroutine,
		this,
		0,
		&threadid_
	);

	DWORD ret;
	while (true)
	{
		ret = MsgWaitForMultipleObjects(
			1, &evStarted_, FALSE, INFINITE, QS_SENDMESSAGE);
		if (ret == WAIT_OBJECT_0)
		{
			break;
		}
		else if (ret == WAIT_OBJECT_0 + 1)
		{
			MSG Msg;
			if (PeekMessage(&Msg, nullptr, 0, 0, PM_REMOVE | PM_QS_SENDMESSAGE))
			{
				TranslateMessage(&Msg);
				DispatchMessage(&Msg);
			}
		}
		else
		{
			assert(0);
		}
	}
}

void Win32UIThread::Close()
{
	if (evStarted_)
	{
		if (WaitForSingleObject(evStarted_, 0) == WAIT_OBJECT_0)
		{
			BOOL ret = PostThreadMessage(threadid_, UTM_QUIT, 0, 0);
			if (ret == FALSE)
			{
				auto err = GetLastError();
				assert(0);
			}
			assert(WaitForSingleObject(hthread_, INFINITE) != WAIT_TIMEOUT);

			SafeClose(&hthread_);
		}
	}
	else
	{
		assert(0);
	}
}

DWORD Win32UIThread::threadroutine(void* instance)
{
	return ((Win32UIThread*)instance)->threadroutine();
}

winrt::Windows::Foundation::IAsyncAction 
Win32UIThread::DispatchAsync(winrt::Windows::System::DispatcherQueue queue)
{
	co_await winrt::resume_foreground(queue);

	res_->InitIndependentRes(true);
}

DWORD Win32UIThread::threadroutine()
{
	winrt::init_apartment(apartment_type::single_threaded);
	winrt::Windows::System::DispatcherQueueController controller{ nullptr };
	DispatcherQueueOptions qopt = {
		sizeof(DispatcherQueueOptions),
		DQTYPE_THREAD_CURRENT, DQTAT_COM_NONE };
	check_hresult(CreateDispatcherQueueController(
		qopt, reinterpret_cast<
		ABI::Windows::System::IDispatcherQueueController**>(put_abi(controller))));
	res_->InitIndependentRes();

	IsGUIThread(TRUE);
	hmsgw_ = ::CreateWindowEx(
		0, 
		Win32Window::msgclass_.lpszClassName, 
		0, 0, 0, 0, 0, 0, 
		HWND_MESSAGE, 
		0, g_hinst, 0);

	OnThreadStarts(this);
	SetEvent(evStarted_);

	handlemsg();

	res_->ReleaseIndependentRes();

	OnThreadEnds(this);
	ResetEvent(evStarted_);
	return 0;
}
