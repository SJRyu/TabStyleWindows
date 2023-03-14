#pragma once

#include <NativeWindows2/windows/TabWindow.h>

namespace NativeWindows
{
	class TabIMGUI : public TabWindow
	{
	public:
		TabIMGUI(Ctab* parent);
		virtual ~TabIMGUI();

		virtual void SetClient() override;

	protected:

	};
}
