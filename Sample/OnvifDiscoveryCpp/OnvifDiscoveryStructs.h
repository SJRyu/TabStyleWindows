#pragma once

#include <string>

namespace OnvifDiscoveryCpp
{
	struct EndpointMetadata
	{
		std::wstring uuid;
		std::wstring xaddr;
		std::wstring name;
	};
}
