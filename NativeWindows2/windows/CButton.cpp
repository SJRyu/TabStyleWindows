#include <pch.h>
#include <NativeWindows2/windows/CButton.h>

CButton::CButton(D2dWinArgs const& args, Color const& bg) :
	D2dWindow1(args), bg_(bg)
{
	assert(parent_ != nullptr);
}

CButton::~CButton()
{
}

LRESULT CButton::OnCreate1(LPCREATESTRUCT createstr)
{
	float w = (float)window_->rect_.width;
	float h = (float)window_->rect_.height;

	visualb_ = AddColorVisual(bg_);
	visualb_.Opacity(0.0f);

	visualf_ = AddD2dVisual(surfacef_, w, h);

	hoveranime_ = refres_->compositor_.CreateScalarKeyFrameAnimation();
	using timespan = std::chrono::duration<int, std::ratio<1, 1000>>;
	hoveranime_.Duration(timespan(400));

	return OnSetup(this);
}

LRESULT CButton::OnSize(WPARAM state, int width, int height)
{
	Redraw(width, height);
	return 0;
}

LRESULT CButton::OnLbtndown(int state, int x, int y)
{
	Status(hover_activate);
	return 0;
}

LRESULT CButton::OnLbtnup(int state, int x, int y)
{
	if (hoverstat_ == hover_activate)
	{
		Status(hover_hovering);
		OnClick(this);
	}
	return 0;
}

LRESULT CButton::OnMousemove(int state, int x, int y)
{
	if (hoverstat_ == hover_inactivate)
	{
		Status(hover_hovering);
	}

	return 0;
}

LRESULT CButton::OnMouseleave()
{
	Status(hover_inactivate);
	return 0;
}

void CButton::Status(hoverstatus stat)
{
	if (hoverstat_ == stat) return;

	switch (stat)
	{
	case hover_hovering:
	{
		TRACKMOUSEEVENT tme = { sizeof(tme) };
		tme.dwFlags = TME_LEAVE;
		tme.hwndTrack = hwnd_;
		tme.dwHoverTime = HOVER_DEFAULT;
		TrackMouseEvent(&tme);

		hoveranime_.InsertKeyFrame(1.0f, 0.40f);
		visualb_.StartAnimation(L"Opacity", hoveranime_);
		break;
	}
	case hover_activate:
	{
		hoveranime_.InsertKeyFrame(1.0f, 0.60f);
		visualb_.StartAnimation(L"Opacity", hoveranime_);
		break;
	}
	case hover_inactivate:
	default:
	{
		hoveranime_.InsertKeyFrame(1.0f, 0.0f);
		visualb_.StartAnimation(L"Opacity", hoveranime_);
		break;
	}
	}

	hoverstat_ = stat;
}

void CButton::Redraw(LONG w, LONG h)
{
	if (w < 1 || h < 1)
	{
		return;
	}

	surfacef_.Resize({ w, h });

	ID2D1DeviceContext* dc;
	BeginDraw(&dc, surfacef_);
	dc->Clear();

	OnRedraw(this, dc, w, h);

	EndDraw(surfacef_);
	dc->Release();
}

void CButton::Bgcolor(Color bg)
{
	visualb_.as<SpriteVisual>().Brush(
	this->Compositor().CreateColorBrush(bg));
}
