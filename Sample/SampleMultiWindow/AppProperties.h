#pragma once

#include <NativeWindows2/windows/WindowProperties.h>

namespace NativeWindows
{
	enum EnumUserMessage1
	{
		UM_OPEN_CAMSETTING = EnumUserMessage::UM_END + 1,
	};

	enum EnumViewProperties
	{
		CAMS_IN_ROW = 2,
		MAX_CAM_COUNT = CAMS_IN_ROW * CAMS_IN_ROW,
	};
}