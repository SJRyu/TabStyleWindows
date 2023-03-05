#pragma once

#include <winrt/Windows.UI.h>
#include <d2d1.h>

namespace NativeWindows
{
	using namespace winrt;
	using namespace Windows::UI;

	struct Win32MsgArgs
	{
		HWND hwnd;
		UINT msg;
		WPARAM wp;
		LPARAM lp;
	};

	enum hoverstatus : int
	{
		hover_none,
		hover_inactivate,
		hover_activate,
		hover_hovering,
	};

	enum EnumUserThreadMessage
	{
		UTM_START = WM_APP,
		UTM_NEWCONTAINER,
		UTM_CLOSECONTAINER,
		UTM_DOCKCONTAINER,
		UTM_SEPARATECONTAINER,
		
		UTM_CLOSE,
		UTM_QUIT,

		UTM_END = UTM_QUIT
	};

	enum EnumUserMessage
	{
		UM_START = WM_USER,
		
		UM_CREATEEX,
		UM_CLOSE,

		UM_BTNCLICKED,
		UM_VBLANK,
		
		UM_CHILD_SIZE,
		UM_CHILD_MOVE,
		UM_VSCROLL_UP,
		UM_VSCROLL_DOWN,
		UM_HSCROLL_UP,
		UM_HSCROLL_DOWN,

		UM_TAB_ADD,
		UM_TAB_REMOVE,
		UM_TAB_ACTIVATE,
		UM_TAB_ACTIVATE1,
		UM_TAB_RESIZE,

		UM_CLIENT_RESIZE,
		UM_ROOT_CHANGED,

		UM_DPICHANGED_AFTERPARENT,

		UM_CREATEDLG,
		UM_CLOSEDLG,
		UM_DLG_XBTN,

		UM_END = UM_DLG_XBTN
	};

	enum EnumUserScrollWP
	{
		HSCROLL_UP,
		HSCROLL_DOWN,
		HSCROLL_REPOS,
		VSCROLL_UP,
		VSCROLL_DOWN,
		VSCROLL_REPOS,
	};

	#define    SC_SZLEFT           (0xF001)  // resize from left
	#define    SC_SZRIGHT          (0xF002)  // resize from right
	#define    SC_SZTOP            (0xF003)  // resize from top
	#define    SC_SZTOPLEFT        (0xF004)  // resize from top left
	#define    SC_SZTOPRIGHT       (0xF005)  // resize from top right
	#define    SC_SZBOTTOM         (0xF006)  // resize from bottom
	#define    SC_SZBOTTOMLEFT     (0xF007)  // resize from bottom left
	#define    SC_SZBOTTOMRIGHT    (0xF008)  // resize from bottom right
	#define    SC_DRAGMOVE         (0xF012)  // move by drag

	inline constexpr LONG containerMinWidth_ = 960;
	inline constexpr Color defcaptioncolor_ = { 0x20, 0x00, 0x00, 0x00 };
	inline constexpr Color defactivecolor_{ 0xFF, 0xFF, 0xFF, 0xFF };
	inline constexpr Color defclientcolor_ = defactivecolor_;
	inline constexpr Color definactivecolor_{ 0xFF, 0xE4, 0xE4, 0xE4 };
	inline constexpr Color defhovercolor_{ 0xFF, 0xF0, 0xF0, 0xF0 };

	inline constexpr D2D1_COLOR_F d2dactivecolor_{ 1.0f, 1.0f, 1.0f, 1.0f };
	inline constexpr D2D1_COLOR_F d2dhovercolor_{ 0.94f, 0.94f, 0.94f, 1.0f };
	inline constexpr D2D1_COLOR_F d2dinactivecolor_{ 0.84f, 0.84f, 0.84f, 1.0f };

	inline constexpr UINT SWP_NOREDRAW1 = SWP_NOREDRAW | SWP_NOSENDCHANGING | SWP_NOCOPYBITS;

	inline constexpr LONG SCROLLBARTHICK = 12;
	inline constexpr LONG SCROLLUNITPIXEL = 30;
};
