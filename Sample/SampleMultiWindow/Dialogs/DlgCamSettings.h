#pragma once

#include <NativeWindows2/cctrl/CDialog.h>
#include <OnvifDiscoveryCpp/OnvifDiscovery.h>

namespace NativeWindows
{
	class VideoView;

	struct CamData
	{
		std::wstring user_;
		std::wstring passwd_;
		std::wstring address_;
		std::wstring uri1_;
		std::wstring uri2_;
		std::wstring resolution_;
		std::wstring bitrate_;
		std::wstring fps_;
	};

	class DlgCamSettings : public CamData, public CDialog
	{
	public:
		DlgCamSettings(VideoView* cam);
		virtual ~DlgCamSettings();

		DlgCamSettings& operator=(CamData const& r)
		{
			auto l = static_cast<CamData*>(this);
			*l = r;
			return *this;
		}

		VideoView* cam_;
		std::wstring caption_;

		virtual BOOL ShowWindow(int cmd = SW_SHOW) override;

	protected:

		HBRUSH hbr_;
		HBITMAP hbmp_;
		HWND hList_;
		HWND hBtnSearch_;
		HWND hEditAddr_;
		HWND hBtnSave_;
		HWND hBtnCancel_;
		
		std::list<OnvifDiscoveryCpp::EndpointMetadata> eplist_;

		void UpdateList();

		virtual LRESULT CALLBACK OnInitDlg() override;
		virtual LRESULT CALLBACK OnCommand(WPARAM wp, LPARAM lp) override;
		virtual LRESULT CALLBACK OnNotify(WPARAM wp, NMHDR* nmhdr) override;
		virtual LRESULT CALLBACK OnSize(WPARAM state, int width, int height) override;
		virtual LRESULT CALLBACK OnPaint(WPARAM wp, LPARAM lp) override;
	};
}
