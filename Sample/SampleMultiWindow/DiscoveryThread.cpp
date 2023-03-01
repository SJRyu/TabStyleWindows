#include <pch.h>
#include <DiscoveryThread.h>

using namespace NativeWindows;

DiscoveryThread::DiscoveryThread()
{
}

DiscoveryThread::~DiscoveryThread()
{
}

DWORD DiscoveryThread::threadroutine()
{
	IsGUIThread(TRUE);

	discovery_ = new OnvifDiscoveryCpp::OnvifDiscovery();
	discovery_->Start();
	SetEvent(evStarted_);

	handlemsg();

	delete discovery_;
	ResetEvent(evStarted_);
	return 0;

	return 0;
}

void DiscoveryThread::handlemsg()
{
	MSG Msg;
	while (GetMessage(&Msg, nullptr, 0, 0))
	{
		switch (Msg.message)
		{
		case UTM_QUIT:
		{
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