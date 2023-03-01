#pragma once

#include <NativeWindows2/Win32UIThread.h>
#include <OnvifDiscoveryCpp/OnvifDiscovery.h>

namespace NativeWindows
{
	class DiscoveryThread : public Win32UIThread
	{
	public:

		DiscoveryThread();
		virtual ~DiscoveryThread();

		inline void GetResolvedEndpoints(
			std::list<OnvifDiscoveryCpp::EndpointMetadata>* outref)
		{
			return discovery_->GetResolvedEndpoints(outref);
		}

	protected:

		OnvifDiscoveryCpp::OnvifDiscovery* discovery_ = nullptr;

		virtual DWORD threadroutine() override;
		virtual void handlemsg() override;
	};
}