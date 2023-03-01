#include <pch.h>
#include <Dialogs/DlgCamSettings.h>
#include <Win32Windows/VideoView.h>
#include <NativeWindows2/windows/CText.h>
#include <MyApp.h>

using namespace NativeWindows;

DlgCamSettings::DlgCamSettings(VideoView* cam) :
	CDialog(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_CAMSETTINGS), cam, cam->thread_),
	cam_(cam)
{
	hbmp_ = ::LoadBitmap(hinst_, MAKEINTRESOURCE(IDB_BITMAP_CAMBG));
	hbr_ = CreatePatternBrush(hbmp_);
}

DlgCamSettings::~DlgCamSettings()
{
	::DeleteObject(hbr_);
	::DeleteObject(hbmp_);
}

void DlgCamSettings::UpdateList()
{
	auto app = MyApp::getInstance();
	app->GetResolvedEndpoints(&eplist_);

	ListView_DeleteAllItems(hList_);

	LVITEM LI;
	ClearMem(&LI);
	LI.mask = LVIF_TEXT;
	for (auto& n : eplist_)
	{
		LI.iSubItem = 0;
		LI.pszText = (LPWSTR)n.name.c_str();
		ListView_InsertItem(hList_, &LI);

		LI.iSubItem = 1;
		LI.pszText = (LPWSTR)n.xaddr.c_str();
		ListView_SetItem(hList_, &LI);

		LI.iItem++;
	}
}

BOOL DlgCamSettings::ShowWindow(int cmd)
{
	if (cmd == SW_SHOW || cmd == SW_SHOWDEFAULT)
	{
		auto& namestring = cam_->namew_->text_;

		auto hEdit = ::GetDlgItem(hwnd_, IDC_EDIT_CAMNAME);
		SetWindowText(hEdit, namestring.c_str());

		UpdateList();
	}
	return ::ShowWindow(hwnd_, cmd);
}

LRESULT DlgCamSettings::OnInitDlg()
{
	caption_ = L"CAM";
	caption_ += std::to_wstring(cam_->channel_);
	caption_ += L" settings";

	::SetWindowText(hwnd_, caption_.c_str());

	hBtnSearch_ = ::GetDlgItem(hwnd_, IDC_BUTTON_CAMSEARCH);
	hEditAddr_ = ::GetDlgItem(hwnd_, IDC_EDIT_CAMADDR);
	hBtnSave_ = ::GetDlgItem(hwnd_, IDC_BUTTON_CAMSAVE);
	hBtnCancel_ = ::GetDlgItem(hwnd_, IDC_BUTTON_CAMCANCEL);

	hList_ = ::GetDlgItem(hwnd_, IDC_LIST_CAMS);
	SendMessage(hList_, LVM_SETEXTENDEDLISTVIEWSTYLE, 0, LVS_EX_FULLROWSELECT);
	
	CRECT<LONG> rc;
	::GetClientRect(hList_, &rc);

	LV_COLUMN col;
	ClearMem(&col);

	col.mask = LVCF_FMT | LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;
	col.cx = rc.width / 3;
	col.pszText = L"Name";
	ListView_InsertColumn(hList_, 0, &col);

	col.cx = rc.width - col.cx;
	col.pszText = L"Address";
	ListView_InsertColumn(hList_, 1, &col);

	return TRUE;
}

LRESULT DlgCamSettings::OnCommand(WPARAM wp, LPARAM lp)
{
	if (lp == (LPARAM)hBtnSearch_)
	{
		if (HIWORD(wp) == BN_CLICKED)
		{
			UpdateList();
		}
	}
	else if (lp == (LPARAM)hBtnSave_)
	{
		if (HIWORD(wp) == BN_CLICKED)
		{
			//SaveData();
			//hide dlg or disable this button?
		}
	}
	else if (lp == (LPARAM)hBtnCancel_)
	{
		if (HIWORD(wp) == BN_CLICKED)
		{
			//RestoreData();
		}
	}

	return FALSE;
}

LRESULT DlgCamSettings::OnNotify(WPARAM wp, NMHDR* nmhdr)
{
	switch (wp)
	{
	case IDC_LIST_CAMS:
	{
		if (nmhdr->code == NM_DBLCLK)
		{
			int sel = ListView_GetNextItem(hList_, -1, LVNI_SELECTED);
			if (sel == -1)
			{
				break;
			}

			wchar_t text[255] = {0};
			ListView_GetItemText(hList_, sel, 1, (LPWSTR)text, 255);
			::SetWindowText(hEditAddr_, text);
			address_ = text;
		}
		break;
	}
	default:
		break;
	}

	return FALSE;
}

LRESULT DlgCamSettings::OnSize(WPARAM state, int width, int height)
{
	CRECT<LONG> rc;
	::GetClientRect(hList_, &rc);

	auto ww = rc.width / 3;
	ListView_SetColumnWidth(hList_, 0, ww);
	ListView_SetColumnWidth(hList_, 1, rc.width - ww);

	return FALSE;
}

LRESULT DlgCamSettings::OnPaint(WPARAM wp, LPARAM lp)
{
	PAINTSTRUCT pstr;
	::BeginPaint(hwnd_, &pstr);
	if (wp == NULL)
	{
		::FillRect(pstr.hdc, &pstr.rcPaint, hbr_);
	}
	::EndPaint(hwnd_, &pstr);

	return TRUE;
}
