#pragma once

#include <NativeWindows2/windows/CTextbase.h>

namespace NativeWindows
{
	// single line text control
	class NATIVEWINDOWS2_API CText : public CTextbase
	{
	public:
		const INT32 maxCharacters_ = 255;

		CText(WinArgs const& args,
			Windows::UI::Color const& bg = Windows::UI::Colors::White());
		CText(WinArgs const& args, TextDrawingEffectBase const& effect,
			Windows::UI::Color const& bg = Windows::UI::Colors::White());
		virtual ~CText();

		virtual void PasteFromClipboard() override;

	protected:

		virtual HRESULT WINAPI InsertTextAt(
			UINT32 position,
			WCHAR const* textToInsert,  // [lengthToInsert]
			IN OUT UINT32* textToInsertLength,
			EditableLayout::CaretFormat* caretFormat = NULL) override;

		virtual void CreateTextFormat() override;

		virtual bool OnVkReturn(UINT32 pos, LPARAM lp) override;
		virtual void OnKeyCharacter(UINT32 charCode) override;

		virtual void CALLBACK OnSize1(WPARAM state, int width, int height) override;
	};
}