#include "pch.h"
#include "DiscoveryWrapper.h"
#include <msclr/lock.h>

using namespace OnvifDiscoveryCpp;

DiscoveryWrapper::DiscoveryWrapper()
{
	client_ = gcnew OnvifDiscoveryDotnet::OnvifWsd();
}

DiscoveryWrapper::~DiscoveryWrapper()
{
	client_ = nullptr;
}

void DiscoveryWrapper::Start()
{
	client_->Start();
}

void DiscoveryWrapper::Stop()
{
	client_->Stop();
}

bool DiscoveryWrapper::IsRunning()
{
	return client_->IsRunning;
}

void DiscoveryWrapper::GetResolvedEndpoints(std::list<EndpointMetadata>* outref)
{
	outref->clear();

	EndpointMetadata ep;
	auto services = client_->CurrentServices1;
	for each (auto n in services)
	{
		ep.uuid = msclr::interop::marshal_as<std::wstring>(n.uuid);
		ep.xaddr = msclr::interop::marshal_as<std::wstring>(n.xaddr);
		ep.name = msclr::interop::marshal_as<std::wstring>(n.name);
		outref->push_back(ep);
	}
}
