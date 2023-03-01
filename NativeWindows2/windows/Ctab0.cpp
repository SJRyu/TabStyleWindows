#include <pch.h>
#include <NativeWindows2/Win32UIThread.h>
#include <NativeWindows2/windows/Ctab.h>
#include <NativeWindows2/windows/WindowProperties.h>
#include <NativeWindows2/windows/ContainerWindow.h>
#include <NativeWindows2/windows/Cbutton.h>
#include <NativeWindows2/windows/TabWindow.h>
//#include <NativeWindows2/windows/NvrTab.h>

Ctab::Ctab(ContainerWindow* container) : 
	D2dWindow1({ 0, 0, container }), refcontainer_(container)
{
	int tabh = ncm_->tabh;
	int vframe = ncm_->vframe;

	tabmaxw_ = tabh * 6;

	btnaddy_ = tabh / 4;
	btnaddr_ = tabh / 2;

	CalcTabwidth(0);

	rect_.left = 0;
	rect_.right = btnaddend_;
	rect_.top = ncm_->taby;
	rect_.bottom = ncm_->captionh;

	wstyle_ |= WS_VISIBLE;
	CreateEx();
}

Ctab::~Ctab()
{
}

LRESULT Ctab::OnCreate1(LPCREATESTRUCT createstr)
{
	RECT rect{ btnaddx_, btnaddy_, btnaddx_ + btnaddr_, btnaddy_ + btnaddr_ };

	btnadd_ = std::make_unique<CEllipseButton>(
		std::forward<D2dWinArgs>({ &rect, 0, this }), std::forward<Color>({ 0xFF, 0x20, 0x80, 0xE0 }));
	btnadd_->OnRedraw = [this](ID2D1DeviceContext* dc, int w, int h)
	{
		FLOAT ww = (FLOAT)w / 2;
		FLOAT hh = (FLOAT)h / 2;

		auto linethick = ncm_->d2e0;

		ID2D1SolidColorBrush* brush;
		dc->CreateSolidColorBrush(D2D1::ColorF(0x606080), &brush);
		dc->DrawLine({ linethick, hh }, { w - linethick, hh },
			brush, linethick);

		dc->DrawLine({ ww, linethick }, { ww, h - linethick },
			brush, linethick);
		brush->Release();
	};

	btnadd_->wstyle_ |= WS_CLIPSIBLINGS;
	btnadd_->CreateEx();

	SetWindowPos(0,
		0, ncm_->taby, 0, 0,
		SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOREDRAW1);

	return 0;
}

void Ctab::CalcTabwidth(int count)
{
	int barmax = refcontainer_->rect_.width - ncm_->sysmenuw - ncm_->sysmenuw;

	int tabend = 0;
	if (count == 0)
	{
		tabwidth_ = (FLOAT)tabmaxw_;
	}
	else
	{
		tabend = (tabmaxw_ * count);
		tabend = min(tabend, barmax);
		tabwidth_ = (FLOAT)(tabend) / count;
	}

	tabend_ = tabend;
	btnaddx_ = tabend + ncm_->vframe;
	btnaddend_ = btnaddx_ + btnaddr_;
}

void Ctab::ReposAlltabs()
{
	btnadd_->SetWindowPos(0, btnaddx_, btnaddy_, 0, 0,
		SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOREDRAW1);

	for (auto n = tabs_.rbegin(); n != tabs_.rend(); n++)
	{
		auto tab = n->get();
		tab->Reposition();
	}
}

LRESULT Ctab::OnDpichangedAfterparent()
{
	int tabh = ncm_->tabh;
	int vframe = ncm_->vframe;

	tabmaxw_ = tabh * 6;

	btnaddy_ = tabh / 4;
	btnaddr_ = tabh / 2;

	CalcTabwidth(tabs_.size());	

	btnadd_->SetWindowPos(0, 0, 0, btnaddr_, btnaddr_,
		SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE | SWP_FRAMECHANGED);

	AnimeWindowPos(0,
		0, ncm_->taby, btnaddend_, ncm_->tabh,
		SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOREDRAW1);

	ReposAlltabs();
	if (current_ >= 0)
	{
		std::next(tabs_.begin(), current_)->get()->ResizeClient();
	}

	return 0;
}

void Ctab::Resize()
{
	CalcTabwidth(tabs_.size());

	if (parent_->isSizing_)
	{
		SetWindowPos(0,
			0, 0, btnaddend_, ncm_->tabh,
			SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOREDRAW1);
	}
	else
	{
		AnimeWindowPos(0,
			0, 0, btnaddend_, ncm_->tabh,
			SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOREDRAW1);

		ReposAlltabs();
		if (current_ >= 0)
		{
			std::next(tabs_.begin(), current_)->get()->ResizeClient();
		}
	}
}

LRESULT Ctab::OnSize(WPARAM state, int width, int height)
{
	RedrawWindow(hwnd_, 0, 0, RDW_VALIDATE | RDW_NOCHILDREN);
	return 0;
}

LRESULT Ctab::OnMove(int x, int y)
{
	MoveVisuals(x, y);
	return 0;
}

LRESULT Ctab::OnCButtonClicked(WPARAM wp, LPARAM lp)
{
	RemoveTab((int)lp);
	return 0;
}

void Ctab::AddTab(TabWindow* tab)
{
	int idx = (int)tabs_.size();

	if (idx < tabmaxn_)
	{
		CalcTabwidth(idx + 1);

		AnimeWindowPos(0,
			0, 0, btnaddend_, ncm_->tabh,
			SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOREDRAW1);

		ReposAlltabs();

		//tabs_.push_back(std::make_unique<TabWindow>(this, idx));
		//tabs_.push_back(std::make_unique<NvrTab>(this, idx));
		tab->position_ = idx;
		tabs_.push_back(std::unique_ptr<TabWindow>(tab));
		tab->CreateEx();
		tab->ShowWindow();
		if (idx)
		{
			std::next(tabs_.begin(), current_)->get()->SetActivate(hover_inactivate);
		}
		current_ = idx;
	}
}


void Ctab::RemoveTab(int idx)
{
	int tabs = (int)tabs_.size() - 1;
	if (tabs < 0) return;
	
	CalcTabwidth(tabs);
	
	//flicker ¹æÁö
	std::unique_ptr<TabWindow> temp;
	auto it = std::next(tabs_.begin(), idx);
	it->swap(temp);
	tabs_.erase(it);
	if (tabs)
	{
		int i = tabs;
		for (auto n = tabs_.rbegin(); n != tabs_.rend(); n++)
		{
			i--;
			auto tab = n->get();
			tab->position_ = i;
			tab->Reposition();
		}

		if (current_ == idx)
		{
			//current_ = min(current_, tabs - 1);
			current_ = tabs - 1;
			std::next(tabs_.begin(), current_)->get()->SetActivate(hover_activate);
		}
		else if (current_ > idx)
		{
			current_--;
		}

		btnadd_->SetWindowPos(0, btnaddx_, btnaddy_, 0, 0,
			SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOREDRAW1);

		AnimeWindowPos(0,
			0, 0, btnaddend_, ncm_->tabh,
			SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOREDRAW1);

		temp->OnTabClose();
	}
	else
	{
		temp->OnTabClose();
		//temp.reset();
		PostMessage(refcontainer_->hwnd_, WM_CLOSE, 0, 0);
		//current_--;
	}
}

void Ctab::ActiveTab(int idx)
{
	if (current_ != idx)
	{
		std::next(tabs_.begin(), idx)->get()->SetActivate(hover_activate);

		std::next(tabs_.begin(), current_)->get()->SetActivate(hover_inactivate);

		current_ = idx;
	}
}

void Ctab::SwapTab(int active, int inactive)
{
	if (active < 0 || inactive < 0)
	{
		return;
	}

	int size = (int)tabs_.size();
	if (active >= size || inactive >= size)
	{
		return;
	}

	auto x = std::next(tabs_.begin(), active);
	auto y = std::next(tabs_.begin(), inactive);

	x->get()->position_ = inactive;
	y->get()->position_ = active;
	y->get()->Reposition();

	x->swap(*y);
	current_ = inactive;
}
