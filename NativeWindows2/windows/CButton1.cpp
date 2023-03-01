#include <pch.h>
#include <NativeWindows2/windows/CButton.h>

CButton1::CButton1(D2dWinArgs const& args) : CButton(args)
{
}

CButton1::~CButton1()
{
}

LRESULT CButton1::OnCreate1(LPCREATESTRUCT createstr)
{
	float w = (float)window_->rect_.width;
	float h = (float)window_->rect_.height;

	visualb_ = AddD2dVisual(surfaceb_, w, h);
	visualb_.Opacity(0.0f);

	visualf_ = AddD2dVisual(surfacef_, w, h);

	hoveranime_ = refres_->compositor_.CreateScalarKeyFrameAnimation();
	using timespan = std::chrono::duration<int, std::ratio<1, 1000>>;
	hoveranime_.Duration(timespan(400));

	Status(hover_inactivate);

	return OnSetup(this);
}

void CButton1::Redraw(LONG w, LONG h)
{
	if (w < 1 || h < 1)
	{
		return;
	}

	surfaceb_.Resize({ w, h });
	surfacef_.Resize({ w, h });

	ID2D1DeviceContext* dc;
	BeginDraw(&dc, surfaceb_);
	dc->Clear();

	OnRedrawBg(this, dc, w, h);

	EndDraw(surfaceb_);
	dc->Release();

	BeginDraw(&dc, surfacef_);
	dc->Clear();

	OnRedraw(this, dc, w, h);

	EndDraw(surfacef_);
	dc->Release();
}

CEllipseButton::CEllipseButton(D2dWinArgs const& args, Color const& bg) :
	CButton(args, bg)
{
}

CEllipseButton::~CEllipseButton()
{
}

void CEllipseButton::Bgcolor(Color bg)
{
	shapeb_.FillBrush(this->Compositor().CreateColorBrush(bg));
}

LRESULT CEllipseButton::OnCreate1(LPCREATESTRUCT createstr)
{
	float w = (float)window_->rect_.width;
	float h = (float)window_->rect_.height;

	auto vb = AddShapeVisual();
	vb.Opacity(0.0f);

	geob_ = this->Compositor().CreateEllipseGeometry();
	geob_.Radius({ w / 2, h / 2 });
	geob_.Center({ w / 2, h / 2 });
	
	shapeb_ = this->Compositor().CreateSpriteShape(geob_);
	shapeb_.FillBrush(this->Compositor().CreateColorBrush(bg_));
	vb.Shapes().Append(shapeb_);
	visualb_ = vb;

	visualf_ = AddD2dVisual(surfacef_, w, h);

	hoveranime_ = refres_->compositor_.CreateScalarKeyFrameAnimation();
	using timespan = std::chrono::duration<int, std::ratio<1, 1000>>;
	hoveranime_.Duration(timespan(400));

	return OnSetup(this);
}

void CEllipseButton::Redraw(LONG w, LONG h)
{
	if (w < 1 || h < 1)
	{
		return;
	}

	float ww = (float)w / 2, hh = (float)h / 2;
	geob_.Center({ ww, hh });
	geob_.Radius({ ww, hh });

	surfacef_.Resize({ w, h });

	ID2D1DeviceContext* dc;
	BeginDraw(&dc, surfacef_);
	dc->Clear();

	OnRedraw(this, dc, w, h);

	EndDraw(surfacef_);
	dc->Release();
}
