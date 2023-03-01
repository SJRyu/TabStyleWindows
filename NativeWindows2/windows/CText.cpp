#include <pch.h>
#include <NativeWindows2/windows/CText.h>
#include <NativeWindows2/directx/TextRenderer.h>
#include <NativeWindows2/directx/EditableLayout.h>

CText::CText(WinArgs const& args, Windows::UI::Color const& bg) :
	CTextbase(args, bg)
{

}

CText::CText(WinArgs const& args,
	TextDrawingEffectBase const& effect, Windows::UI::Color const& bg) :
	CTextbase(args, effect, bg)
{
	
}

CText::~CText()
{

}

void CText::OnSize1(WPARAM state, int width, int height)
{
	auto fsize = GetFontSize(fontSize_);
	textLayout_->SetFontSize(fsize, MakeDWriteTextRange(0));

	UINT32 currentPos = caretPosition_ + caretPositionOffset_;
	if (currentPos > 0)
	{
		--currentPos; // Always adopt the trailing properties.
	}
	textLayout_->GetFontSize(currentPos, &caretFormat_.fontSize);
}

void CText::CreateTextFormat()
{
	float w = (float)rect_.width, h = (float)rect_.height;
	auto fsize = GetFontSize(fontSize_);

	assert(refres_->dwriteFactory_->CreateTextFormat(
		L"Segoe UI", nullptr,
		fontAxis_, sizeof(fontAxis_) / sizeof(DWRITE_FONT_AXIS_VALUE),
		fsize, 
		L"ko_KR", 
		textFormat_.put()) == S_OK);

	IDWriteTextLayout* layout_ = nullptr;
	assert(refres_->dwriteFactory_->CreateTextLayout(
		text_.c_str(), (UINT32)text_.size(),
		textFormat_.get(),
		w, h,
		&layout_) == S_OK);
	layout_->QueryInterface(textLayout_.put());
	layout_->Release();
	
	textLayout_->SetWordWrapping(DWRITE_WORD_WRAPPING_NO_WRAP);
	DWRITE_TRIMMING trim{ DWRITE_TRIMMING_GRANULARITY_NONE, 0, 0 };
	HR(textLayout_->SetTrimming(&trim, nullptr));
}


void CText::PasteFromClipboard()
{
	// Pastes text from clipboard at current caret position.
	DeleteSelection();
	UINT32 characterCount = 0;

	// Copy Unicode text from clipboard.

	if (OpenClipboard(hwnd_))
	{
		HGLOBAL hClipboardData = GetClipboardData(CF_UNICODETEXT);

		if (hClipboardData != NULL)
		{
			// Get text and size of text.
			size_t byteSize = GlobalSize(hClipboardData);
			void* memory = GlobalLock(hClipboardData); // [byteSize] in bytes
			const wchar_t* text = reinterpret_cast<const wchar_t*>(memory);
			characterCount = static_cast<UINT32>(wcsnlen(text, byteSize / sizeof(wchar_t)));

			if (memory != NULL)
			{
				std::wstring rstring(text);
				rstring.erase(std::remove_if(rstring.begin(), rstring.end(),
					[](wchar_t c)
					{
						if (c == L'\f' || c == L'\n' || c == L'\r' || c == L'\t' || c == L'\v')
						{
							return true;
						}
						else return false;
					}), rstring.end());

				text = rstring.c_str();
				characterCount = (UINT32)rstring.size();
				
				// Insert the text at the current position.
				InsertTextAt(
					caretPosition_ + caretPositionOffset_,
					text,
					&characterCount
				);
				GlobalUnlock(hClipboardData);
			}
		}
		CloseClipboard();
	}

	SetSelection(SetSelectionModeRightChar, characterCount, true);
}

HRESULT CText::InsertTextAt(
	UINT32 position,
	WCHAR const* textToInsert,  // [lengthToInsert]
	IN OUT UINT32* textToInsertLength,
	EditableLayout::CaretFormat* caretFormat)
{
	HRESULT hr = S_OK;

	INT32 len = (INT32)*textToInsertLength;
	INT32 space = maxCharacters_ - ((INT32)position + len);
	if (space < 0)
	{
		len = std::max(0, len + space);
		*textToInsertLength = len;
	}

	if (*textToInsertLength)
	{
		len = (INT32)text_.size();
		space = maxCharacters_ - (len + *textToInsertLength);
		if (space < 0)
		{
			text_ = text_.substr(0, len + space);
		}

		auto layout = textLayout_.detach();
		hr = layoutEditor_->InsertTextAt(
			&layout,
			text_, position,
			textToInsert, *textToInsertLength,
			caretFormat);
		textLayout_.attach(layout);
		bRedrawText_ = true;
	}

	return hr;
}

bool CText::OnVkReturn(UINT32 pos, LPARAM lp)
{
	::SetFocus(nullptr);
	return false;
}

void CText::OnKeyCharacter(UINT32 charCode)
{
	// Inserts text characters.
	// tabkey is not allowed.
	if (charCode >= 0x20)
	{
		// Replace any existing selection.
		DeleteSelection();

		// Convert the UTF32 character code from the Window message to UTF16,
		// yielding 1-2 code-units. Then advance the caret position by how
		// many code-units were inserted.

		UINT32 charsLength = 1;
		wchar_t chars[2] = { static_cast<wchar_t>(charCode), 0 };

		// If above the basic multi-lingual plane, split into
		// leading and trailing surrogates.
		if (charCode > 0xFFFF)
		{
			// From http://unicode.org/faq/utf_bom.html#35
			chars[0] = wchar_t(0xD800 + (charCode >> 10) - (0x10000 >> 10));
			chars[1] = wchar_t(0xDC00 + (charCode & 0x3FF));
			charsLength++;
		}

		InsertTextAt(
			caretPosition_ + caretPositionOffset_,
			chars, &charsLength,
			&caretFormat_);
		SetSelection(SetSelectionModeRight, charsLength, false, false);
		RefreshView();
	}
}

