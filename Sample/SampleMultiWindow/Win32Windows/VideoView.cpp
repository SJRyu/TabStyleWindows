#include <pch.h>
#include <Win32Windows/VideoView.h>
#include <NativeWindows2/Win32UIThread.h>
#include <MyApp.h>
#include <Win32Windows/NvrWindow.h>
#include <Win32Windows/NvrTab.h>
#include <Dialogs/DlgCamSettings.h>
#include <NativeWindows2/windows/CText.h>
#include <NativeWindows2/directx/TextDrawingEffect.h>

using namespace NativeWindows;

VideoView::VideoView(WinArgs const& args, int chn, int pos) :
	VideoWindow(args), nvrw_((NvrWindow*)args.parent), channel_(chn), position_(pos)
{
	tab_ = nvrw_->tab_;

	wstyle_ |= WS_VISIBLE | WS_BORDER;
}

VideoView::~VideoView()
{
}

LRESULT VideoView::OnCreate1(LPCREATESTRUCT createstr)
{
	cmenu_ = ::GetSubMenu(MyApp::hmenu_, 0);

	auto margin = DpiVal(nameMargin_);
	RECT rc{ margin, 0, rect_.width / 2 - margin, DpiVal(nameh_) + margin };

	TextDrawingEffectBase effect;
	effect.color_ = D2D1::ColorF(D2D1::ColorF::Black);
	effect.olcolor_ = D2D1::ColorF(D2D1::ColorF::White, 0.8f);
	effect.olthickness_ = 2.0f;

	namew_ = wmake_unique<CText>(
		WinArgs{ &rc, 0, this }, effect, Windows::UI::Color{ 0, 0, 0, 0 }
	);
	namew_->fontAxis_[0].value = (FLOAT)DWRITE_FONT_WEIGHT_DEMI_BOLD;
	namew_->fontSize_ = nameh_;
	namew_->text_ = L"My Spot " + std::to_wstring(channel_);

	namew_->OnSetup = [](CTextbase* sender)
	{
		sender->rootv_.Opacity(0.5f);
	};

	namew_->CreateEx();
	namew_->ShowWindow();

	return 0;
}

void VideoView::OnClose1()
{
	if (dlgsettings_)
	{
		dlgsettings_->WaitClose();
		delete dlgsettings_;
	}
	namew_.reset();

	ReleaseRtspc();
}

LRESULT VideoView::OnSize(WPARAM state, int width, int height)
{
	auto margin = DpiVal(nameMargin_);
	namew_->SetWindowPos(0, 
		margin, 0, width / 2 - margin, DpiVal(nameh_) + margin,
		SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOREDRAW1);

	return 0;
}
LRESULT VideoView::OnLbtndown(int state, int x, int y)
{

	return 0;
}
LRESULT VideoView::OnLbtndouble(int state, int x, int y)
{

	return 0;
}

BOOL VideoView::OnContextMenu(HWND hwnd, int xpos, int ypos)
{
	::TrackPopupMenu(cmenu_,
		TPM_LEFTALIGN | TPM_RIGHTBUTTON,
		xpos, ypos, 0, hwnd_, nullptr);

	return TRUE;
}

LRESULT VideoView::OnCommand(WPARAM wp, LPARAM lp)
{
	if (lp == 0)
	{
		switch (wp)
		{
		case ID_CAMERA_CAMSETTINGS:
		{
			if (dlgsettings_ == nullptr)
			{
				dlgsettings_ = new DlgCamSettings(this);
				dlgsettings_->CreateCDlg();
			}
			dlgsettings_->ShowWindow();
			return FALSE;
		}
		default :
			break;
		}
	}
	return TRUE;
}

LRESULT VideoView::UserMsgHandler(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_TIMER:
	{
		::PostMessage(tab_->hwnd_, UM_OPEN_CAMSETTING, (WPARAM)channel_, 0);
		return 0;
	}
	case UM_DLG_XBTN:
	{
		dlgsettings_->ShowWindow(SW_HIDE);
		return 0;
	}
	default:
		break;
	}
	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

void VideoView::RtspcOnConnected()
{
	dbg_msg("RtspcOnConnected");
}

void VideoView::RtspcOnDisconnectFromServer()
{
	dbg_msg("RtspcOnDisconnectFromServer");
}

unsigned char* VideoView::RtspcOnGetBuffer()
{
	// RTSP is not implemented in this sample.
	return NULL;
}

void VideoView::RtspcOnFrame(unsigned char* frame, unsigned int size, timeval& ts)
{
	//Decoder is not implemented in this sample
}
