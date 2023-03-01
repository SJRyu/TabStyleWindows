#include "pch.h"
#include "OnvifDiscovery.h"
#include "DiscoveryWrapper.h"

using namespace OnvifDiscoveryCpp;

OnvifDiscovery::OnvifDiscovery()
{
	wrapper_ = new DiscoveryWrapper();
}

OnvifDiscovery::~OnvifDiscovery()
{
	delete wrapper_;
}

bool OnvifDiscovery::IsRunning()
{
	return wrapper_->IsRunning();
}

void OnvifDiscovery::Start()
{
	wrapper_->Start();
}

void OnvifDiscovery::Stop()
{
	wrapper_->Stop();
}

void OnvifDiscovery::GetResolvedEndpoints(std::list<EndpointMetadata>* outref)
{
	return wrapper_->GetResolvedEndpoints(outref);
}
