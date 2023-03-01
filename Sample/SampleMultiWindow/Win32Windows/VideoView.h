#pragma once

#include <NativeWindows2/windows/VideoWindow.h>
#include <WinSock2.h>

//RTSP and Decoder is not implemented in this sample.
//Below class is just a sample for the future RTSPclient.
class __declspec(novtable) RtspcCallback
{
public:
	virtual void CALLBACK RtspcOnConnected() = 0;

	virtual void CALLBACK RtspcOnDisconnectFromServer() = 0;

	virtual unsigned char* CALLBACK RtspcOnGetBuffer() = 0;

	virtual void CALLBACK RtspcOnFrame(unsigned char* frame, unsigned int size, timeval& ts) = 0;
};

namespace NativeWindows
{
	class NvrWindow;
	class NvrTab;
	class DlgCamSettings;
	class CText;

	class VideoView : public VideoWindow, public RtspcCallback
	{
	public:

		VideoView(WinArgs const& args, int chn, int pos);
		virtual ~VideoView();

		NvrWindow* nvrw_;
		NvrTab* tab_;
		wunique_ptr<CText> namew_;

		int channel_;
		int position_;

		inline void ReleaseRtspc()
		{
			//RTSP is not implemented in this sample.
		}

	protected:

		static constexpr int nameh_ = 30;
		static constexpr int nameMargin_ = 4;

		HMENU cmenu_ = nullptr;
		DlgCamSettings* dlgsettings_ = nullptr;

		void CALLBACK RtspcOnConnected() override;
		void CALLBACK RtspcOnDisconnectFromServer()override;
		unsigned char* CALLBACK RtspcOnGetBuffer() override;
		void CALLBACK RtspcOnFrame(unsigned char* frame, unsigned int size, timeval& ts) override;

		virtual LRESULT CALLBACK OnCreate1(LPCREATESTRUCT createstr) override;
		virtual void CALLBACK OnClose1() override;
		virtual LRESULT CALLBACK OnSize(WPARAM state, int width, int height) override;
		virtual LRESULT CALLBACK OnLbtndown(int state, int x, int y) override;
		virtual LRESULT CALLBACK OnLbtndouble(int state, int x, int y) override;

		virtual BOOL CALLBACK OnContextMenu(HWND hwnd, int xpos, int ypos) override;
		virtual LRESULT CALLBACK OnCommand(WPARAM wp, LPARAM lp) override;
		virtual LRESULT CALLBACK UserMsgHandler(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) override;
	};
}
