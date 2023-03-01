#include <pch.h>
#include <NativeWindows2/windows/CTextbase.h>
#include <NativeWindows2/directx/TextRenderer.h>
#include <NativeWindows2/directx/EditableLayout.h>
#define _USE_MATH_DEFINES
#include <math.h>
#if 0
// winrt clipboard might better?
#include <winrt/Windows.ApplicationModel.DataTransfer.h>
using namespace winrt::Windows::ApplicationModel::DataTransfer;
#endif

namespace
{
	// Private helper functions.
	inline D2D1::Matrix3x2F& Cast(DWRITE_MATRIX& matrix)
	{
		// DWrite's matrix, D2D's matrix, and GDI's XFORM
		// are all compatible.
		return *reinterpret_cast<D2D1::Matrix3x2F*>(&matrix);
	}

	inline DWRITE_MATRIX& Cast(D2D1::Matrix3x2F& matrix)
	{
		return *reinterpret_cast<DWRITE_MATRIX*>(&matrix);
	}

	inline int RoundToInt(float x)
	{
		return static_cast<int>(floor(x + .5));
	}

	inline double DegreesToRadians(float degrees)
	{
		return degrees * M_PI * 2.0f / 360.0f;
	}

	inline float GetDeterminant(DWRITE_MATRIX const& matrix)
	{
		return matrix.m11 * matrix.m22 - matrix.m12 * matrix.m21;
	}

	void ComputeInverseMatrix(
		DWRITE_MATRIX const& matrix,
		OUT DWRITE_MATRIX& result
	)
	{
		// Used for hit-testing, mouse scrolling, panning, and scroll bar sizing.

		float invdet = 1.f / GetDeterminant(matrix);
		result.m11 = matrix.m22 * invdet;
		result.m12 = -matrix.m12 * invdet;
		result.m21 = -matrix.m21 * invdet;
		result.m22 = matrix.m11 * invdet;
		result.dx = (matrix.m21 * matrix.dy - matrix.dx * matrix.m22) * invdet;
		result.dy = (matrix.dx * matrix.m12 - matrix.m11 * matrix.dy) * invdet;
	}

	D2D1_POINT_2F GetPageSize(IDWriteTextLayout* textLayout)
	{
		// Use the layout metrics to determine how large the page is, taking
		// the maximum of the content size and layout's maximal dimensions.

		DWRITE_TEXT_METRICS textMetrics;
		textLayout->GetMetrics(&textMetrics);

		float width = (std::max)(textMetrics.layoutWidth, textMetrics.left + textMetrics.width);
		float height = (std::max)(textMetrics.layoutHeight, textMetrics.height);

		D2D1_POINT_2F pageSize = { width, height };
		return pageSize;
	}
}

CTextbase::CTextbase(WinArgs const& args, Windows::UI::Color const& bg) :
	D2dWindow(args), bgcolor_(bg)
{
	effectBase_.color_ = D2D1::ColorF(D2D1::ColorF::Black);
	effectBase_.olcolor_ = D2D1::ColorF(D2D1::ColorF::White);
	effectBase_.olthickness_ = 0;
}

CTextbase::CTextbase(WinArgs const& args, 
	TextDrawingEffectBase const& effect, Windows::UI::Color const& bg) :
	D2dWindow(args), bgcolor_(bg)
{
	effectBase_ = effect;
}


CTextbase::~CTextbase()
{

}

void CTextbase::CopyToClipboard()
{
	// Copies selected text to clipboard.
	DWRITE_TEXT_RANGE selectionRange = GetSelectionRange();
	if (selectionRange.length <= 0)
		return;

	// Open and empty existing contents.
	if (OpenClipboard(hwnd_))
	{
		if (EmptyClipboard())
		{
			// Allocate room for the text
			size_t byteSize = sizeof(wchar_t) * (selectionRange.length + 1);
			HGLOBAL hClipboardData = GlobalAlloc(GMEM_DDESHARE | GMEM_ZEROINIT, byteSize);

			if (hClipboardData != NULL)
			{
				void* memory = GlobalLock(hClipboardData);  // [byteSize] in bytes

				if (memory != NULL)
				{
					// Copy text to memory block.
					const wchar_t* text = text_.c_str();
					memcpy(memory, &text[selectionRange.startPosition], byteSize);
					GlobalUnlock(hClipboardData);

					if (SetClipboardData(CF_UNICODETEXT, hClipboardData) != NULL)
					{
						hClipboardData = NULL; // system now owns the clipboard, so don't touch it.
					}
				}
				GlobalFree(hClipboardData); // free if failed
			}
		}
		CloseClipboard();
	}
}

void CTextbase::PasteFromClipboard()
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

HRESULT CTextbase::InsertTextAt(
	UINT32 position,
	WCHAR const* textToInsert,  // [lengthToInsert]
	IN OUT UINT32* textToInsertLength,
	EditableLayout::CaretFormat* caretFormat)
{
	auto layout = textLayout_.detach();
	auto hr = layoutEditor_->InsertTextAt(
		&layout, 
		text_, position, 
		textToInsert, *textToInsertLength, 
		caretFormat);
	textLayout_.attach(layout);
	bRedrawText_ = true;
	return hr;
}

HRESULT CTextbase::RemoveTextAt(
	UINT32 position,
	UINT32 lengthToRemove)
{
	auto layout = textLayout_.detach();
	auto hr = layoutEditor_->RemoveTextAt(
		&layout, text_, position, lengthToRemove);
	textLayout_.attach(layout);
	bRedrawText_ = true;
	return hr;
}

HRESULT CTextbase::ClearText()
{
	auto layout = textLayout_.detach();
	auto hr = layoutEditor_->Clear(&layout, text_);
	textLayout_.attach(layout);
	bRedrawText_ = true;
	return hr;
}

void CTextbase::UpdateSystemCaret(D2D1_RECT_F const& rect)
{
	// Moves the system caret to a new position.

	// Although we don't actually use the system caret (drawing our own
	// instead), this is important for accessibility, so the magnifier
	// can follow text we type. The reason we draw our own directly
	// is because intermixing DirectX and GDI content (the caret) reduces
	// performance.

	// Gets the current caret position (in untransformed space).

	if (GetFocus() != hwnd_) // Only update if we have focus.
		return;

	// Update the caret's location, rounding to nearest integer so that
	// it lines up with the text selection.

	float width = (rect.right - rect.left);
	float height = (rect.bottom - rect.top);

	int intX = RoundToInt(rect.left);
	int intY = RoundToInt(rect.top);
	int intWidth = RoundToInt(width);
	int intHeight = RoundToInt(height);

	::CreateCaret(hwnd_, NULL, intWidth, intHeight);
	::SetCaretPos(intX, intY);

	// Don't actually call ShowCaret. It's enough to just set its position.
}

void CTextbase::GetCaretRect(OUT D2D1_RECT_F* rect)
{
	*rect = { 0, 0, 0, 0 };

	// Gets the current caret position (in untransformed space).
	if (textLayout_ == NULL)
		return;

	// Translate text character offset to point x,y.
	DWRITE_HIT_TEST_METRICS caretMetrics;
	float caretX, caretY;

	textLayout_->HitTestTextPosition(
		caretPosition_,
		caretPositionOffset_ > 0, // trailing if nonzero, else leading edge
		&caretX,
		&caretY,
		&caretMetrics
	);

	// If a selection exists, draw the caret using the
	// line size rather than the font size.
	DWRITE_TEXT_RANGE selectionRange = GetSelectionRange();
	if (selectionRange.length > 0)
	{
		UINT32 actualHitTestCount = 1;
		textLayout_->HitTestTextRange(
			caretPosition_,
			0, // length
			0, // x
			0, // y
			&caretMetrics,
			1,
			&actualHitTestCount
		);

		caretY = caretMetrics.top;
	}

	//i want floor
	caretThickness_ = ncm_->dpiy / 96;

	// Return the caret rect, untransformed.
	rect->left = caretX;
	rect->right = rect->left + caretThickness_;
	rect->top = caretY;
	rect->bottom = caretY + caretMetrics.height;
}

void CTextbase::UpdateCaretFormat()
{
	UINT32 currentPos = caretPosition_ + caretPositionOffset_;
	if (currentPos > 0)
	{
		--currentPos; // Always adopt the trailing properties.
	}

	// Get the family name
	caretFormat_.fontFamilyName[0] = '\0';
	textLayout_->GetFontFamilyName(currentPos, &caretFormat_.fontFamilyName[0], ARRAYSIZE(caretFormat_.fontFamilyName));

	// Get the locale
	caretFormat_.localeName[0] = '\0';
	textLayout_->GetLocaleName(currentPos, &caretFormat_.localeName[0], ARRAYSIZE(caretFormat_.localeName));

	// Get the remaining attributes...
	textLayout_->GetFontWeight(currentPos, &caretFormat_.fontWeight);
	textLayout_->GetFontStyle(currentPos, &caretFormat_.fontStyle);
	textLayout_->GetFontStretch(currentPos, &caretFormat_.fontStretch);
	textLayout_->GetFontSize(currentPos, &caretFormat_.fontSize);
	textLayout_->GetUnderline(currentPos, &caretFormat_.hasUnderline);
	textLayout_->GetStrikethrough(currentPos, &caretFormat_.hasStrikethrough);

	// Get the current color.
	IUnknown* drawingEffect = NULL;
	textLayout_->GetDrawingEffect(currentPos, &drawingEffect);
	if (drawingEffect != NULL)
	{
		TextDrawingEffect* effect = reinterpret_cast<TextDrawingEffect*>(drawingEffect);
		caretFormat_.color = effect->color_;
		SafeRelease(&effect);
	}
	else
	{
		caretFormat_.color = {0, 0, 0, 0};
	}
}

DWRITE_TEXT_RANGE CTextbase::GetSelectionRange()
{
	// Returns a valid range of the current selection,
   // regardless of whether the caret or anchor is first.

	UINT32 caretBegin = caretAnchor_;
	UINT32 caretEnd = caretPosition_ + caretPositionOffset_;
	if (caretBegin > caretEnd)
		std::swap(caretBegin, caretEnd);

	// Limit to actual text length.
	UINT32 textLength = static_cast<UINT32>(text_.size());
	caretBegin = std::min(caretBegin, textLength);
	caretEnd = std::min(caretEnd, textLength);

	DWRITE_TEXT_RANGE textRange = { caretBegin, caretEnd - caretBegin };
	return textRange;
}

void CTextbase::AlignCaretToNearestCluster(bool isTrailingHit, bool skipZeroWidth)
{
	// Uses hit-testing to align the current caret position to a whole cluster,
	// rather than residing in the middle of a base character + diacritic,
	// surrogate pair, or character + UVS.

	DWRITE_HIT_TEST_METRICS hitTestMetrics;
	float caretX, caretY;

	// Align the caret to the nearest whole cluster.
	textLayout_->HitTestTextPosition(
		caretPosition_,
		false,
		&caretX,
		&caretY,
		&hitTestMetrics
	);

	// The caret position itself is always the leading edge.
	// An additional offset indicates a trailing edge when non-zero.
	// This offset comes from the number of code-units in the
	// selected cluster or surrogate pair.
	caretPosition_ = hitTestMetrics.textPosition;
	caretPositionOffset_ = (isTrailingHit) ? hitTestMetrics.length : 0;

	// For invisible, zero-width characters (like line breaks
	// and formatting characters), force leading edge of the
	// next position.
	if (skipZeroWidth && hitTestMetrics.width == 0)
	{
		caretPosition_ += caretPositionOffset_;
		caretPositionOffset_ = 0;
	}
}

void CTextbase::GetLineMetrics(
	OUT std::vector<DWRITE_LINE_METRICS>& lineMetrics
)
{
	// Retrieves the line metrics, used for caret navigation, up/down and home/end.

	DWRITE_TEXT_METRICS textMetrics;
	textLayout_->GetMetrics(&textMetrics);

	lineMetrics.resize(textMetrics.lineCount);
	textLayout_->GetLineMetrics(&lineMetrics.front(), textMetrics.lineCount, &textMetrics.lineCount);
}

void CTextbase::GetLineFromPosition(
	const DWRITE_LINE_METRICS* lineMetrics, // [lineCount]
	UINT32 lineCount,
	UINT32 textPosition,
	OUT UINT32* lineOut,
	OUT UINT32* linePositionOut
)
{
	// Given the line metrics, determines the current line and starting text
	// position of that line by summing up the lengths. When the starting
	// line position is beyond the given text position, we have our line.

	UINT32 line = 0;
	UINT32 linePosition = 0;
	UINT32 nextLinePosition = 0;
	for (; line < lineCount; ++line)
	{
		linePosition = nextLinePosition;
		nextLinePosition = linePosition + lineMetrics[line].length;
		if (nextLinePosition > textPosition)
		{
			// The next line is beyond the desired text position,
			// so it must be in the current line.
			break;
		}
	}
	*linePositionOut = linePosition;
	*lineOut = std::min(line, lineCount - 1);
	return;
}

bool CTextbase::SetSelection(
	SetSelectionMode moveMode, UINT32 advance, bool extendSelection, bool updateCaretFormat)
{
	// Moves the caret relatively or absolutely, optionally extending the
	// selection range (for example, when shift is held).

	UINT32 line = UINT32_MAX; // current line number, needed by a few modes
	UINT32 absolutePosition = caretPosition_ + caretPositionOffset_;
	UINT32 oldAbsolutePosition = absolutePosition;
	UINT32 oldCaretAnchor = caretAnchor_;

	switch (moveMode)
	{
	case SetSelectionModeLeft:
		caretPosition_ += caretPositionOffset_;
		if (caretPosition_ > 0)
		{
			--caretPosition_;
			AlignCaretToNearestCluster(false, true);

			// special check for CR/LF pair
			absolutePosition = caretPosition_ + caretPositionOffset_;
			if (absolutePosition >= 1
				&& absolutePosition < text_.size()
				&& text_[absolutePosition - 1] == '\r'
				&& text_[absolutePosition] == '\n')
			{
				caretPosition_ = absolutePosition - 1;
				AlignCaretToNearestCluster(false, true);
			}
		}
		break;

	case SetSelectionModeRight:
		caretPosition_ = absolutePosition;
		AlignCaretToNearestCluster(true, true);

		// special check for CR/LF pair
		absolutePosition = caretPosition_ + caretPositionOffset_;
		if (absolutePosition >= 1
			&& absolutePosition < text_.size()
			&& text_[absolutePosition - 1] == '\r'
			&& text_[absolutePosition] == '\n')
		{
			caretPosition_ = absolutePosition + 1;
			AlignCaretToNearestCluster(false, true);
		}
		break;

	case SetSelectionModeLeftChar:
		caretPosition_ = absolutePosition;
		caretPosition_ -= std::min(advance, absolutePosition);
		caretPositionOffset_ = 0;
		break;

	case SetSelectionModeRightChar:
		caretPosition_ = absolutePosition + advance;
		caretPositionOffset_ = 0;
		{
			// Use hit-testing to limit text position.
			DWRITE_HIT_TEST_METRICS hitTestMetrics;
			float caretX, caretY;

			textLayout_->HitTestTextPosition(
				caretPosition_,
				false,
				&caretX,
				&caretY,
				&hitTestMetrics
			);
			caretPosition_ = std::min(caretPosition_, hitTestMetrics.textPosition + hitTestMetrics.length);
		}
		break;

	case SetSelectionModeUp:
	case SetSelectionModeDown:
	{
		// Retrieve the line metrics to figure out what line we are on.
		std::vector<DWRITE_LINE_METRICS> lineMetrics;
		GetLineMetrics(lineMetrics);

		UINT32 linePosition;
		GetLineFromPosition(
			&lineMetrics.front(),
			static_cast<UINT32>(lineMetrics.size()),
			caretPosition_,
			&line,
			&linePosition
		);

		// Move up a line or down
		if (moveMode == SetSelectionModeUp)
		{
			if (line <= 0)
				break; // already top line
			line--;
			linePosition -= lineMetrics[line].length;
		}
		else
		{
			linePosition += lineMetrics[line].length;
			line++;
			if (line >= lineMetrics.size())
				break; // already bottom line
		}

		// To move up or down, we need three hit-testing calls to determine:
		// 1. The x of where we currently are.
		// 2. The y of the new line.
		// 3. New text position from the determined x and y.
		// This is because the characters are variable size.

		DWRITE_HIT_TEST_METRICS hitTestMetrics;
		float caretX, caretY, dummyX;

		// Get x of current text position
		textLayout_->HitTestTextPosition(
			caretPosition_,
			caretPositionOffset_ > 0, // trailing if nonzero, else leading edge
			&caretX,
			&caretY,
			&hitTestMetrics
		);

		// Get y of new position
		textLayout_->HitTestTextPosition(
			linePosition,
			false, // leading edge
			&dummyX,
			&caretY,
			&hitTestMetrics
		);

		// Now get text position of new x,y.
		BOOL isInside, isTrailingHit;
		textLayout_->HitTestPoint(
			caretX,
			caretY,
			&isTrailingHit,
			&isInside,
			&hitTestMetrics
		);

		caretPosition_ = hitTestMetrics.textPosition;
		caretPositionOffset_ = isTrailingHit ? (hitTestMetrics.length > 0) : 0;
	}
	break;

	case SetSelectionModeLeftWord:
	case SetSelectionModeRightWord:
	{
		// To navigate by whole words, we look for the canWrapLineAfter
		// flag in the cluster metrics.

		// First need to know how many clusters there are.
		std::vector<DWRITE_CLUSTER_METRICS> clusterMetrics;
		UINT32 clusterCount;
		textLayout_->GetClusterMetrics(NULL, 0, &clusterCount);

		if (clusterCount == 0)
			break;

		// Now we actually read them.
		clusterMetrics.resize(clusterCount);
		textLayout_->GetClusterMetrics(&clusterMetrics.front(), clusterCount, &clusterCount);

		caretPosition_ = absolutePosition;

		UINT32 clusterPosition = 0;
		UINT32 oldCaretPosition = caretPosition_;

		if (moveMode == SetSelectionModeLeftWord)
		{
			// Read through the clusters, keeping track of the farthest valid
			// stopping point just before the old position.
			caretPosition_ = 0;
			caretPositionOffset_ = 0; // leading edge
			for (UINT32 cluster = 0; cluster < clusterCount; ++cluster)
			{
				clusterPosition += clusterMetrics[cluster].length;
				if (clusterMetrics[cluster].canWrapLineAfter)
				{
					if (clusterPosition >= oldCaretPosition)
						break;

					// Update in case we pass this point next loop.
					caretPosition_ = clusterPosition;
				}
			}
		}
		else // SetSelectionModeRightWord
		{
			// Read through the clusters, looking for the first stopping point
			// after the old position.
			for (UINT32 cluster = 0; cluster < clusterCount; ++cluster)
			{
				UINT32 clusterLength = clusterMetrics[cluster].length;
				caretPosition_ = clusterPosition;
				caretPositionOffset_ = clusterLength; // trailing edge
				if (clusterPosition >= oldCaretPosition && clusterMetrics[cluster].canWrapLineAfter)
					break; // first stopping point after old position.

				clusterPosition += clusterLength;
			}
		}
	}
	break;

	case SetSelectionModeHome:
	case SetSelectionModeEnd:
	{
		// Retrieve the line metrics to know first and last position
		// on the current line.
		std::vector<DWRITE_LINE_METRICS> lineMetrics;
		GetLineMetrics(lineMetrics);

		GetLineFromPosition(
			&lineMetrics.front(),
			static_cast<UINT32>(lineMetrics.size()),
			caretPosition_,
			&line,
			&caretPosition_
		);

		caretPositionOffset_ = 0;
		if (moveMode == SetSelectionModeEnd)
		{
			// Place the caret at the last character on the line,
			// excluding line breaks. In the case of wrapped lines,
			// newlineLength will be 0.
			UINT32 lineLength = lineMetrics[line].length - lineMetrics[line].newlineLength;
			caretPositionOffset_ = std::min(lineLength, 1u);
			caretPosition_ += lineLength - caretPositionOffset_;
			AlignCaretToNearestCluster(true);
		}
	}
	break;

	case SetSelectionModeFirst:
		caretPosition_ = 0;
		caretPositionOffset_ = 0;
		break;

	case SetSelectionModeAll:
		caretAnchor_ = 0;
		extendSelection = true;
		__fallthrough;

	case SetSelectionModeLast:
		caretPosition_ = UINT32_MAX;
		caretPositionOffset_ = 0;
		AlignCaretToNearestCluster(true);
		break;

	case SetSelectionModeAbsoluteLeading:
		caretPosition_ = advance;
		caretPositionOffset_ = 0;
		break;

	case SetSelectionModeAbsoluteTrailing:
		caretPosition_ = advance;
		AlignCaretToNearestCluster(true);
		break;
	}

	absolutePosition = caretPosition_ + caretPositionOffset_;

	if (!extendSelection)
		caretAnchor_ = absolutePosition;

	bool caretMoved = (absolutePosition != oldAbsolutePosition)
		|| (caretAnchor_ != oldCaretAnchor);

	if (caretMoved)
	{
		// update the caret formatting attributes
		if (updateCaretFormat)
			UpdateCaretFormat();

		bRedrawCaret_ = true;

		D2D1_RECT_F rect;
		GetCaretRect(&rect);
		UpdateSystemCaret(rect);

		D2D1_RECT_F viewRc
		{
			viewx_, viewy_,
			viewx_ + (float)rect_.width, viewy_ + (float)rect_.height
		};

		if (rect.left < viewRc.left)
		{
			viewx_ = rect.left;
			bRedrawText_ = true;
		}
		else if (rect.right > viewRc.right)
		{
			viewx_ += (rect.right - viewRc.right);
			bRedrawText_ = true;;
		}

		if (rect.top < viewRc.top)
		{
			viewy_ = rect.top;
			bRedrawText_ = true;
		}
		else if (rect.bottom > viewRc.bottom)
		{
			viewy_ += (rect.bottom - viewRc.bottom);
			bRedrawText_ = true;;
		}
	}

	return caretMoved;
}

bool CTextbase::SetSelectionFromPoint(float x, float y, bool extendSelection)
{
	// Returns the text position corresponding to the mouse x,y.
	// If hitting the trailing side of a cluster, return the
	// leading edge of the following text position.

	x += viewx_;
	y += viewy_;

	BOOL isTrailingHit;
	BOOL isInside;
	DWRITE_HIT_TEST_METRICS caretMetrics;

	textLayout_->HitTestPoint(
		x,
		y,
		&isTrailingHit,
		&isInside,
		&caretMetrics
	);

	//dbg_msg("hit %f,%f position=%d(%s)", x, y, caretMetrics.textPosition, isTrailingHit ? "true" : "false");

	// Update current selection according to click or mouse drag.
	return SetSelection(
		isTrailingHit ? SetSelectionModeAbsoluteTrailing : SetSelectionModeAbsoluteLeading,
		caretMetrics.textPosition,
		extendSelection
	);
}

void CTextbase::RedrawCaret()
{
	if (rect_.width < 1 || rect_.height < 1)
	{
		return;
	}

	D2D1_RECT_F caretRect;
	GetCaretRect(&caretRect);

	caretsurface_.Resize({ rect_.width, rect_.height });

	ID2D1DeviceContext* dc;
	ID2D1SolidColorBrush* brush;
	BeginDraw(&dc, caretsurface_);
	dc->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Black, 0.9f), &brush);
	dc->SetAntialiasMode(D2D1_ANTIALIAS_MODE_ALIASED);

	D2D1::Matrix3x2F oldmat;
	dc->GetTransform(&oldmat);
	auto mat = oldmat;
	mat.dx -= viewx_;
	mat.dy -= viewy_;
	dc->SetTransform(mat);
	dc->Clear();

	// draw caret
	dc->FillRectangle(caretRect, brush);

	// draw selection
	// Determine actual number of hit-test ranges
	DWRITE_TEXT_RANGE caretRange = GetSelectionRange();
	if (caretRange.length > 0)
	{
		UINT32 actualHitTestCount = 0;

		textLayout_->HitTestTextRange(
			caretRange.startPosition,
			caretRange.length,
			0, // x
			0, // y
			NULL,
			0, // metrics count
			&actualHitTestCount
		);

		if (actualHitTestCount)
		{
			// Allocate enough room to return all hit-test metrics.
			std::vector<DWRITE_HIT_TEST_METRICS> hitTestMetrics(actualHitTestCount);

			textLayout_->HitTestTextRange(
				caretRange.startPosition,
				caretRange.length,
				0, // x
				0, // y
				&hitTestMetrics[0],
				static_cast<UINT32>(hitTestMetrics.size()),
				&actualHitTestCount
			);

			brush->SetColor(D2D1::ColorF(D2D1::ColorF::LightBlue, 0.5f));
			for (size_t i = 0; i < actualHitTestCount; ++i)
			{
				const DWRITE_HIT_TEST_METRICS& htm = hitTestMetrics[i];
				D2D1_RECT_F highlightRect =
				{
					htm.left,
					htm.top,
					(htm.left + htm.width),
					(htm.top + htm.height)
				};

				dc->FillRectangle(highlightRect, brush);
			}
		}
	}

	EndDraw(caretsurface_);
	brush->Release();
	dc->Release();
}

void CTextbase::RedrawText()
{
	auto tw = rect_.width;
	auto th = rect_.height;

	if (tw < 1 || th < 1) 
		return;

	textsurface_.Resize({ tw, th });

	auto dc = BeginDraw1(textsurface_);
	//dc->SetAntialiasMode(D2D1_ANTIALIAS_MODE_PER_PRIMITIVE);
	//dc->SetTextAntialiasMode(D2D1_TEXT_ANTIALIAS_MODE_CLEARTYPE);

	D2D1::Matrix3x2F oldmat;
	dc->GetTransform(&oldmat);
	auto mat = oldmat;
	mat.dx -= viewx_;
	mat.dy -= viewy_;
	dc->SetTransform(mat);
	dc->Clear();

	textLayout_->Draw(nullptr, textRenderer_.get(), 0, 0);

	dc->SetTransform(oldmat);
	EndDraw1(textsurface_);
}

float CTextbase::GetFontSize(float heightInPixel)
{
	return floor((heightInPixel * 72 * ncm_->dpiy) / (96 * 96));
}

void CTextbase::CreateTextFormat()
{
	float w = (float)rect_.width, h = (float)rect_.height;
	auto fsize = GetFontSize(fontSize_);

	assert(refres_->dwriteFactory_->CreateTextFormat(
		L"Arial", nullptr,
		fontAxis_, sizeof(fontAxis_) / sizeof(DWRITE_FONT_AXIS_VALUE),
		fsize, 
		L"", 
		textFormat_.put()) == S_OK);

	IDWriteTextLayout* layout_ = nullptr;
	assert(refres_->dwriteFactory_->CreateTextLayout(
		text_.c_str(), (UINT32)text_.size(),
		textFormat_.get(),
		w, h,
		&layout_) == S_OK);
	layout_->QueryInterface(textLayout_.put());
	layout_->Release();
}

LRESULT CTextbase::OnCreate1(LPCREATESTRUCT createstr)
{
	CreateBitmapDc();
	//auto antia = bitmapdc_->GetAntialiasMode();
	//auto antit = bitmapdc_->GetTextAntialiasMode();
	//bitmapdc_->SetAntialiasMode(D2D1_ANTIALIAS_MODE_ALIASED);
	//bitmapdc_->SetTextAntialiasMode(D2D1_TEXT_ANTIALIAS_MODE_GRAYSCALE);
	//FLOAT dx, dy;
	//bitmapdc_->GetDpi(&dx, &dy);
	//auto mode = bitmapdc_->GetUnitMode();
	//bitmapdc_->SetUnitMode(D2D1_UNIT_MODE_PIXELS);
	//bitmapdc_->SetDpi(96, 96);

	float w = (float)rect_.width, h = (float)rect_.height;
	vieww_ = w, viewh_ = h;

	bgvisual_ = AddColorVisual(bgcolor_);
	textvisual_ = AddD2dVisual(textsurface_, w, h);
	caretvisual_ = AddD2dVisual(caretsurface_, w, h);
	
	caretvisual_.IsVisible(false);

	assert(TextRenderer::Create(
		refres_->dwriteFactory_.get(), refres_->d2dFactory_.get(), bitmapdc_.get(),
		this, textRenderer_.put(), &effectBase_) == S_OK);

	CreateTextFormat();
	UpdateCaretFormat();

	layoutEditor_ = std::make_unique<EditableLayout>(refres_->dwriteFactory_.get());

	OnSetup(this);

	return 0;
}

LRESULT CTextbase::OnSize(WPARAM state, int width, int height)
{
	vieww_ = (float)width, viewh_ = (float)height;
	textLayout_->SetMaxWidth(vieww_);
	textLayout_->SetMaxHeight(viewh_);

	OnSize1(state, width, height);
	
	// create이후에 사이즈가 변경이 되지 않으면(wm_size는 걸린다), wm_paint가 발생 안함.
	RedrawText();
	RedrawCaret();
	::ValidateRect(hwnd_, nullptr);
	return 0;
}

LRESULT CTextbase::OnDpichangedAfterparent()
{
	return 0;
}

LRESULT CTextbase::OnLbtndown(int state, int x, int y)
{
	::SetCapture(hwnd_);

	//Start dragging selection.
	currentlySelecting_ = true;

	if (SetSelectionFromPoint((float)x, (float)y, (state & MK_SHIFT)))
	{
		RefreshView();
	}
	return 0;
}

LRESULT CTextbase::OnLbtnup(int state, int x, int y)
{
	::ReleaseCapture();
	currentlySelecting_ = false;
	return 0;
}

LRESULT CTextbase::OnMousemove(int state, int x, int y)
{
	if (currentlySelecting_)
	{
		// Drag current selection.
		if (SetSelectionFromPoint((float)x, (float)y, true))
		{
			RefreshView();
		}
	}
	return 0;
}

bool CTextbase::DeleteSelection()
{
	// Deletes selection.
	DWRITE_TEXT_RANGE selectionRange = GetSelectionRange();
	if (selectionRange.length <= 0)
		return false;

	RemoveTextAt(selectionRange.startPosition, selectionRange.length);
	SetSelection(SetSelectionModeAbsoluteLeading, selectionRange.startPosition, false);
	return true;
}

void CTextbase::OnIMECharacter(UINT32 charCode)
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
	SetSelection(SetSelectionModeRight, charsLength, true, false);
	RefreshView();
}

void CTextbase::OnKeyCharacter(UINT32 charCode)
{
	// Inserts text characters.
	// Allow normal characters and tabs
	if (charCode >= 0x20 || charCode == 9)
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

bool CTextbase::OnVkReturn(UINT32 pos, LPARAM lp)
{
	// Insert CR/LF pair
	DeleteSelection();
	UINT32 len = 2;
	InsertTextAt(pos, L"\r\n", &len, &caretFormat_);
	SetSelection(SetSelectionModeAbsoluteLeading, pos + len, false, false);

	return true;
}

void CTextbase::OnKeyPress(UINT32 keyCode, LPARAM lp)
{
	// Handles caret navigation and special presses that
	// do not generate characters.

	bool heldShift = (GetKeyState(VK_SHIFT) & 0x80) != 0;
	bool heldControl = (GetKeyState(VK_CONTROL) & 0x80) != 0;

	UINT32 absolutePosition = caretPosition_ + caretPositionOffset_;
	bool bUpdate = false;
	switch (keyCode)
	{
	case VK_RETURN:
		bUpdate = OnVkReturn(absolutePosition, lp);
		break;

	case VK_BACK:
		// Erase back one character (less than a character though).
		// Since layout's hit-testing always returns a whole cluster,
		// we do the surrogate pair detection here directly. Otherwise
		// there would be no way to delete just the diacritic following
		// a base character.

		if (absolutePosition != caretAnchor_)
		{
			// delete the selected text
			DeleteSelection();
			bUpdate = true;
		}
		else if (absolutePosition > 0)
		{
			UINT32 count = 1;
			// Need special case for surrogate pairs and CR/LF pair.
			if (absolutePosition >= 2
				&& absolutePosition <= text_.size())
			{
				wchar_t charBackOne = text_[absolutePosition - 1];
				wchar_t charBackTwo = text_[absolutePosition - 2];
				if ((IsLowSurrogate(charBackOne) && IsHighSurrogate(charBackTwo))
					|| (charBackOne == '\n' && charBackTwo == '\r'))
				{
					count = 2;
				}
			}
			SetSelection(SetSelectionModeLeftChar, count, false);
			RemoveTextAt(caretPosition_, count);
			bUpdate = true;
		}
		break;

	case VK_DELETE:
		// Delete following cluster.
		if (absolutePosition != caretAnchor_)
		{
			// Delete all the selected text.
			DeleteSelection();
			bUpdate = true;
		}
		else
		{
			DWRITE_HIT_TEST_METRICS hitTestMetrics;
			float caretX, caretY;

			// Get the size of the following cluster.
			textLayout_->HitTestTextPosition(
				absolutePosition,
				false,
				&caretX,
				&caretY,
				&hitTestMetrics
			);

			RemoveTextAt(hitTestMetrics.textPosition, hitTestMetrics.length);
			SetSelection(SetSelectionModeAbsoluteLeading, hitTestMetrics.textPosition, false);
			bUpdate = true;
		}
		break;

	case VK_LEFT: // seek left one cluster
		bUpdate = SetSelection(heldControl ? SetSelectionModeLeftWord : SetSelectionModeLeft, 1, heldShift);
		break;

	case VK_RIGHT: // seek right one cluster
		bUpdate = SetSelection(heldControl ? SetSelectionModeRightWord : SetSelectionModeRight, 1, heldShift);
		break;

	case VK_UP: // up a line
		bUpdate = SetSelection(SetSelectionModeUp, 1, heldShift);
		break;

	case VK_DOWN: // down a line
		bUpdate = SetSelection(SetSelectionModeDown, 1, heldShift);
		break;

	case VK_HOME: // beginning of line
		bUpdate = SetSelection(heldControl ? SetSelectionModeFirst : SetSelectionModeHome, 0, heldShift);
		break;

	case VK_END: // end of line
		bUpdate = SetSelection(heldControl ? SetSelectionModeLast : SetSelectionModeEnd, 0, heldShift);
		break;

	case 'C':
		if (heldControl)
		{
			CopyToClipboard();
		}
		break;

	case VK_INSERT:
		if (heldControl)
		{
			CopyToClipboard();
		}
		else if (heldShift)
		{
			PasteFromClipboard();
			bUpdate = true;
		}
		break;

	case 'V':
		if (heldControl)
		{
			PasteFromClipboard();
			bUpdate = true;
		}
		break;

	case 'X':
		if (heldControl)
		{
			CopyToClipboard();
			DeleteSelection();
			bUpdate = true;
		}
		break;

	case 'A':
		if (heldControl)
			bUpdate = SetSelection(SetSelectionModeAll, 0, true);
		break;
	case VK_ESCAPE:
		bUpdate = OnVkEscape(keyCode, lp);
		break;
	case VK_TAB:
		bUpdate = OnVkTab(keyCode, lp);
		break; // want tabs
	default:
		break;
	}

	if (bUpdate) RefreshView();
}

void CTextbase::RefreshView()
{
	::RedrawWindow(hwnd_, 0, 0, RDW_INTERNALPAINT);
}

LRESULT CTextbase::OnPaint()
{
	if (bRedrawText_)
	{
		RedrawText();
		bRedrawText_ = false;
	}
	if (bRedrawCaret_)
	{
		RedrawCaret();
		bRedrawCaret_ = false;
	}
	::ValidateRect(hwnd_, nullptr);
	return 0;
}

LRESULT CTextbase::UserMsgHandler(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_SETFOCUS:
	{
		caretvisual_.IsVisible(true);
		D2D1_RECT_F rect;
		GetCaretRect(&rect);
		UpdateSystemCaret(rect);
		return 0;
	}
	case WM_KILLFOCUS:
	{
		caretvisual_.IsVisible(false);
		::DestroyCaret();
		return 0;
	}
	case WM_KEYDOWN:
	{
		OnKeyPress((UINT)wParam, lParam);
		return 0;
	}
	case WM_CHAR:
	{
		OnKeyCharacter(static_cast<UINT>(wParam));
		return 0;
	}
	case WM_IME_COMPOSITION:
	{
		if (lParam & GCS_COMPSTR)
		{
			if (lParam & CS_INSERTCHAR)
			{
				OnIMECharacter((UINT)wParam);
			}
			else
			{
				DeleteSelection();
				RefreshView();
			}
		}
		else if (lParam & GCS_RESULTSTR)
		{
			::SendMessage(hWnd, WM_CHAR, wParam, lParam);
		}
		return 0;
	}
	default:
		break;
	}
	return ::DefWindowProc(hWnd, uMsg, wParam, lParam);
}
