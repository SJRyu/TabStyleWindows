#pragma once

#include <NativeWindows2/windows/D2dWindow.h>
#include <NativeWindows2/directx/EditableLayout.h>
#include <NativeWindows2/directx/TextDrawingEffect.h>

namespace NativeWindows
{
	class TextRenderer;

	class NATIVEWINDOWS2_API CTextbase : public D2dWindow
	{
	public:
		enum SetSelectionMode
		{
			SetSelectionModeLeft,               // cluster left
			SetSelectionModeRight,              // cluster right
			SetSelectionModeUp,                 // line up
			SetSelectionModeDown,               // line down
			SetSelectionModeLeftChar,           // single character left (backspace uses it)
			SetSelectionModeRightChar,          // single character right
			SetSelectionModeLeftWord,           // single word left
			SetSelectionModeRightWord,          // single word right
			SetSelectionModeHome,               // front of line
			SetSelectionModeEnd,                // back of line
			SetSelectionModeFirst,              // very first position
			SetSelectionModeLast,               // very last position
			SetSelectionModeAbsoluteLeading,    // explicit position (for mouse click)
			SetSelectionModeAbsoluteTrailing,   // explicit position, trailing edge
			SetSelectionModeAll                 // select all text
		};

		CTextbase() = delete;
		CTextbase(WinArgs const& args,
			Windows::UI::Color const& bg = Windows::UI::Colors::White());
		CTextbase(WinArgs const& args, TextDrawingEffectBase const& effect,
			Windows::UI::Color const& bg = Windows::UI::Colors::White());
		virtual ~CTextbase();

		std::wstring text_ = L"test test";
		FLOAT fontSize_ = 20.0f;
		DWRITE_FONT_AXIS_VALUE fontAxis_[4] =
		{
			{DWRITE_FONT_AXIS_TAG_WEIGHT, (FLOAT)DWRITE_FONT_WEIGHT_REGULAR},
			{DWRITE_FONT_AXIS_TAG_WIDTH, (FLOAT)100},
			{DWRITE_FONT_AXIS_TAG_SLANT, (FLOAT)0},
			{DWRITE_FONT_AXIS_TAG_ITALIC, (FLOAT)0}
		};

		Windows::UI::Color bgcolor_;
		SpriteVisual bgvisual_{ nullptr };
		SpriteVisual textvisual_{ nullptr };
		SpriteVisual caretvisual_{ nullptr };

		std::function<void(CTextbase*)> OnSetup = [](CTextbase*) {};

		IDWriteTextLayout4* GetLayout() { return textLayout_.get(); }

		void GetCaretRect(OUT D2D1_RECT_F* rect);
		void UpdateSystemCaret(D2D1_RECT_F const& rect);

		void UpdateCaretFormat();
		bool SetSelection(SetSelectionMode moveMode, UINT32 advance, 
			bool extendSelection, bool updateCaretFormat = true);
		DWRITE_TEXT_RANGE GetSelectionRange();
		bool DeleteSelection();

		void CopyToClipboard();
		virtual void PasteFromClipboard(); // need to change
		
		EditableLayout::CaretFormat& GetCaretFormat() { return caretFormat_; }
		inline UINT32 GetCaretPosition() { return caretPosition_ + caretPositionOffset_; }

		virtual void RedrawText();
		virtual void RedrawCaret(); //and selection

	protected:

		CompositionDrawingSurface textsurface_{ nullptr };
		CompositionDrawingSurface caretsurface_{ nullptr };
		//CompositionVirtualDrawingSurface textsurface_{ nullptr };

		com_ptr<IDWriteTextFormat3> textFormat_;
		com_ptr<IDWriteTextLayout4> textLayout_;
		com_ptr<TextRenderer> textRenderer_;
		com_ptr<TextDrawingEffect> textEffect_;
		std::unique_ptr<EditableLayout> layoutEditor_;

		LONG caretThickness_ = 1;

		float vieww_, viewh_;
		float viewx_ = 0, viewy_ = 0;

		EditableLayout::CaretFormat caretFormat_;
		UINT32 caretAnchor_ = 0;
		UINT32 caretPosition_ = 0;
		UINT32 caretPositionOffset_ = 0;

		bool currentlySelecting_ = false;
		float previousMouseX_ = 0;
		float previousMouseY_ = 0;

		bool bRedrawCaret_ = false;
		bool bRedrawText_ = false;

		TextDrawingEffectBase effectBase_;

		virtual float GetFontSize(float heightInPixel);

		virtual void CreateTextFormat();
		bool SetSelectionFromPoint(float x, float y, bool extendSelection);
		void AlignCaretToNearestCluster(bool isTrailingHit = false, bool skipZeroWidth = false);
		void GetLineMetrics(OUT std::vector<DWRITE_LINE_METRICS>& lineMetrics);
		void GetLineFromPosition(
			const DWRITE_LINE_METRICS* lineMetrics,   // [lineCount]
			UINT32 lineCount,
			UINT32 textPosition,
			OUT UINT32* lineOut,
			OUT UINT32* linePositionOut
		);

		virtual HRESULT WINAPI InsertTextAt(
			UINT32 position,
			WCHAR const* textToInsert,  // [lengthToInsert]
			IN OUT UINT32* textToInsertLength,
			EditableLayout::CaretFormat* caretFormat = NULL);

		HRESULT WINAPI RemoveTextAt(
			UINT32 position,
			UINT32 lengthToRemove);

		HRESULT WINAPI ClearText();

		virtual bool OnVkReturn(UINT32 pos, LPARAM lp);
		virtual bool OnVkEscape(UINT32 pos, LPARAM lp) 
		{ 
			::SetFocus(nullptr);
			return false; 
		};
		virtual bool OnVkTab(UINT32 pos, LPARAM lp) { return false; };

		virtual void OnIMECharacter(UINT32 charCode);
		virtual void OnKeyCharacter(UINT32 charCode);
		void OnKeyPress(UINT32 keyCode, LPARAM lp);

		void RefreshView();

		virtual LRESULT CALLBACK OnCreate1(LPCREATESTRUCT createstr) override;
		virtual LRESULT CALLBACK OnSize(WPARAM state, int width, int height) override;
		virtual void CALLBACK OnSize1(WPARAM state, int width, int height) {};
		virtual LRESULT CALLBACK OnDpichangedAfterparent() override;
		virtual LRESULT CALLBACK OnLbtndown(int state, int x, int y) override;
		virtual LRESULT CALLBACK OnLbtnup(int state, int x, int y) override;
		virtual LRESULT CALLBACK OnMousemove(int state, int x, int y) override;
		virtual LRESULT CALLBACK OnPaint() override;

		virtual LRESULT CALLBACK UserMsgHandler(
			HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) override;

	};
}
