#pragma once

#ifdef ONVIFDISCOVERY_EXPORTS
#define ONVIFDISCOVERY_API __declspec(dllexport)
#else
#define ONVIFDISCOVERY_API __declspec(dllimport)
#endif

#include <list>
#include <OnvifDiscoveryCpp/OnvifDiscoveryStructs.h>

namespace OnvifDiscoveryCpp
{
	class DiscoveryWrapper;

	class ONVIFDISCOVERY_API OnvifDiscovery
	{
	public:

		OnvifDiscovery();
		virtual ~OnvifDiscovery();

		void Start();
		void Stop();
		bool IsRunning();

		void GetResolvedEndpoints(std::list<EndpointMetadata>* outref);

	private:

		DiscoveryWrapper* wrapper_;
	};
}
