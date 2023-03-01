#pragma once

#include <list>
#include <OnvifDiscoveryCpp/OnvifDiscoveryStructs.h>

using namespace System;

namespace OnvifDiscoveryCpp
{
	class DiscoveryWrapper
	{
	public:
		
		DiscoveryWrapper();
		~DiscoveryWrapper();

		void Start();
		void Stop();
		bool IsRunning();

		void GetResolvedEndpoints(std::list<EndpointMetadata>* outref);

	private:
		
		msclr::auto_gcroot<OnvifDiscoveryDotnet::OnvifWsd^> client_;
	};
}
