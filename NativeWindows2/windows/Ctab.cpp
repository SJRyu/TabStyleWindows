#include <pch.h>
#include <NativeWindows2/Win32UIThread.h>
#include <NativeWindows2/windows/Ctab.h>
#include <NativeWindows2/windows/WindowProperties.h>
#include <NativeWindows2/windows/ContainerWindow.h>
#include <NativeWindows2/windows/Cbutton.h>
#include <NativeWindows2/windows/TabWindow.h>
#include <NativeWindows2/windows/ClientWindow.h>

Ctab::Ctab(ContainerWindow* container) : 
	D2dWindow({ 0, 0, container }), refcontainer_(container)
{
	tabmaxn_ = container->tabmaxn_;
}

Ctab::~Ctab()
{
}

VOID Ctab::OnClose1()
{
	tabs_.clear();
	btnadd_.reset();
}

void Ctab::CreateEx()
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

	Win32Window::CreateEx();
}

LRESULT Ctab::OnCreate1(LPCREATESTRUCT createstr)
{
	//AddColorVisual({ 0xFF, 0, 0 });

	RECT rect{ btnaddx_, btnaddy_, btnaddx_ + btnaddr_, btnaddy_ + btnaddr_ };

	btnadd_ = wmake_unique<CEllipseButton>(
		std::forward<D2dWinArgs>({ &rect, 0, this }), std::forward<Color>({ 0xFF, 0x20, 0x80, 0xE0 }));
	btnadd_->OnRedraw = [](CButton* btn, ID2D1DeviceContext* dc, int w, int h)
	{
		FLOAT ww = (FLOAT)w / 2;
		FLOAT hh = (FLOAT)h / 2;

		auto linethick = btn->ncm_->d2e0;

		ID2D1SolidColorBrush* brush;
		dc->CreateSolidColorBrush(D2D1::ColorF(0x606080), &brush);
		dc->DrawLine({ linethick, hh }, { w - linethick, hh },
			brush, linethick);

		dc->DrawLine({ ww, linethick }, { ww, h - linethick },
			brush, linethick);
		brush->Release();
	};

	//btnadd_->wstyle_ |= WS_CLIPSIBLINGS;
	btnadd_->CreateEx();

	OnSetup(this);

	SetWindowPos(0,
		0, ncm_->taby, 0, 0,
		SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOREDRAW1);

	return 0;
}

void Ctab::CalcTabwidth(int count)
{
	int barmax = refcontainer_->rect_.width - ncm_->sysmenuw - ncm_->sysmenuw;
	barmax = std::max(barmax, 0);

	int tabend = 0;
	if (count == 0)
	{
		tabwidth_ = (FLOAT)tabmaxw_;
	}
	else
	{
		tabend = (tabmaxw_ * count);
		tabend = std::min(tabend, barmax);
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

	SetWindowPos(0,
		0, ncm_->taby, btnaddend_, ncm_->tabh,
		SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOREDRAW1);

	ReposAlltabs();
	if (tab_)
	{
		tab_->ResizeClient();
	}

	return 0;
}

void Ctab::Resize(BOOL resizing)
{
	CalcTabwidth(tabs_.size());

	if (resizing)
	{
		SetWindowPos(0,
			0, 0, btnaddend_, ncm_->tabh,
			SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOREDRAW1);
	}
	else
	{
		SetWindowPos(0,
			0, 0, btnaddend_, ncm_->tabh,
			SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOREDRAW1);

		ReposAlltabs();
		if (tab_)
		{
			tab_->ResizeClient();
		}
	}
}

LRESULT Ctab::OnSize(WPARAM state, int width, int height)
{
	return 0;
}

LRESULT Ctab::OnMove(int x, int y)
{
	return 0;
}

LRESULT Ctab::OnCButtonClicked(WPARAM wp, LPARAM lp)
{
	
	return 0;
}

LRESULT Ctab::OnNcHitTest(LPARAM lparam)
{
	return HTTRANSPARENT;
}

LRESULT Ctab::UserMsgHandler(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (hWnd == hwnd_)
	{
		switch (uMsg)
		{
		case UM_TAB_RESIZE:
		{
			Resize((BOOL)wParam);
			return 0;
		}
		case UM_TAB_ADD:
		{
			return AddTab((TabWindow*)wParam);
		}
		case UM_TAB_REMOVE:
		{
			return RemoveTab((TabWindow*)wParam);
		}
		case UM_TAB_ACTIVATE:
		{
			return ActiveTab((int)wParam);
		}
		case UM_TAB_ACTIVATE1:
		{
			return ActiveTab((TabWindow*)wParam);
		}
		default:
		{
			break;
		}
		}
	}
	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

bool Ctab::InsertTab(TabWindow* tab)
{
	int idx = (int)tabs_.size();
	
	if (idx < tabmaxn_)
	{
		CalcTabwidth(idx + 1);

		SetWindowPos(0,
			0, 0, btnaddend_, ncm_->tabh,
			SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOREDRAW1);

		tab->position_ = idx;
		auto old = tab_;
		current_ = idx;
		tab_ = tab;

		assert(tab->SetParent(this) != NULL);

		tab->parentv_ = topv_.Children();
		tab->parentv_.InsertAtTop(tab->rootv_);
		tab->client_->SetWindowPos(HWND_TOP,
			0, ncm_->captionh,
			refcontainer_->rect_.width,
			refcontainer_->rect_.height - ncm_->captionh,
			SWP_NOACTIVATE | SWP_SHOWWINDOW | SWP_NOREDRAW1);

		tabs_.push_back(wunique_ptr<TabWindow>(tab));
		if (old)
		{
			old->SetActivate(hover_inactivate);
		}
		
		ReposAlltabs();

		return true;
	}
	else
	{
		return false;
	}
}

bool Ctab::MoveoutTab(TabWindow* tab)
{
	int tabn = (int)tabs_.size() - 1;

	CalcTabwidth(tabn);

	wunique_ptr<TabWindow> temp;
	auto it = std::find_if(tabs_.begin(), tabs_.end(),
		[tab](wunique_ptr<TabWindow>& el)
		{
			return (el.get() == tab);
		}
	);
	if (it == tabs_.end())
	{
		dbg_msg("tab already removed");
		return false;
	}

	it->swap(temp);
	tabs_.erase(it);

	temp->StopDragging();
	temp->parentv_.Remove(tab->rootv_);
	temp->parentv_ = nullptr;
	int idx = temp->position_;
	temp.release();

	if (tabn)
	{
		int i = tabn;
		for (auto n = tabs_.rbegin(); n != tabs_.rend(); n++)
		{
			i--;
			auto tab = n->get();
			tab->position_ = i;
			tab->Reposition();
		}

		if (current_ == idx)
		{
			current_ = tabn - 1;
			tab_ = std::next(tabs_.begin(), current_)->get();
			tab_->SetActivate(hover_activate);
		}
		else if (current_ > idx)
		{
			current_--;
		}

		btnadd_->SetWindowPos(0, btnaddx_, btnaddy_, 0, 0,
			SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOREDRAW1);

		SetWindowPos(0,
			0, 0, btnaddend_, ncm_->tabh,
			SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOREDRAW1);
	}
	else
	{
		current_ = -1;
		tab_ = nullptr;

		refcontainer_->ShowWindow(SW_HIDE);
		PostMessage(refcontainer_->hwnd_, UM_CLOSE, 0, 0);
	}
	return true;
}

bool Ctab::AddTab(TabWindow* tab)
{
	/**
	* tab에서 tab을 만들경우 재귀되는 것에 주의.
	**/
	int idx = (int)tabs_.size();

	if (idx < tabmaxn_)
	{
		CalcTabwidth(idx + 1);

		SetWindowPos(0,
			0, 0, btnaddend_, ncm_->tabh,
			SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOREDRAW1);

		ReposAlltabs();

		tab->position_ = idx;
		auto old = tab_;
		current_ = idx;
		tab_ = tab;

		/**
		* RSJ: very dirty, incomplete code, better block mouseinput instead.
		**/
		if (old) old->StopDragging();

		tabs_.push_back(wunique_ptr<TabWindow>(tab));
		tab->CreateEx();
		tab->ShowWindow();
		if (old)
		{
			old->SetActivate(hover_inactivate);
		}

		return true;
	}
	else
	{
		dbg_msg("add-tab failed");
		delete tab;
		return false;
	}
}


bool Ctab::RemoveTab(int idx)
{
	int tabs = (int)tabs_.size() - 1;
	if (tabs < 0) return false;
	
	tab_->StopDragging();

	CalcTabwidth(tabs);

	//flicker 방지
	wunique_ptr<TabWindow> temp;
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
			current_ = tabs - 1;
			tab_ = std::next(tabs_.begin(), current_)->get();
			tab_->SetActivate(hover_activate);
		}
		else if (current_ > idx)
		{
			current_--;
		}

		btnadd_->SetWindowPos(0, btnaddx_, btnaddy_, 0, 0,
			SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOREDRAW1);

		SetWindowPos(0,
			0, 0, btnaddend_, ncm_->tabh,
			SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOREDRAW1);
	}
	else
	{
		//PostMessage로 종료하는 것으로 인해 문제 발생가능...
		current_ = -1;
		tab_ = nullptr;

		temp.reset();
		PostMessage(refcontainer_->hwnd_, UM_CLOSE, 0, 0);
	}

	return true;
}

bool Ctab::RemoveTab(TabWindow* tab)
{
	int tabs = (int)tabs_.size() - 1;
	if (tabs < 0)
	{
		dbg_msg("tab empty");
		return false;
	}

	tab_->StopDragging();

	CalcTabwidth(tabs);

	wunique_ptr<TabWindow> temp;
	auto it = std::find_if(tabs_.begin(), tabs_.end(),
		[tab](wunique_ptr<TabWindow>& el)
		{
			return (el.get() == tab);
		}
	);
	if (it == tabs_.end())
	{
		dbg_msg("tab already removed");
		return false;
	}

	it->swap(temp);
	tabs_.erase(it);

	int idx = temp->position_;
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
			current_ = tabs - 1;
			tab_ = std::next(tabs_.begin(), current_)->get();
			tab_->SetActivate(hover_activate);
		}
		else if (current_ > idx)
		{
			current_--;
		}

		btnadd_->SetWindowPos(0, btnaddx_, btnaddy_, 0, 0,
			SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOREDRAW1);

		SetWindowPos(0,
			0, 0, btnaddend_, ncm_->tabh,
			SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOREDRAW1);
	}
	else
	{
		//PostMessage로 종료하는 것으로 인해 문제 발생가능...
		current_ = -1;
		tab_ = nullptr;

		temp.reset();
		PostMessage(refcontainer_->hwnd_, UM_CLOSE, 0, 0);
	}
	return true;
}

bool Ctab::ActiveTab(int idx)
{
	if (current_ != idx)
	{
		auto activeone = std::next(tabs_.begin(), idx)->get();
		activeone->SetActivate(hover_activate);

		tab_->SetActivate(hover_inactivate);
		tab_ = activeone;

		current_ = idx;

		return true;
	}
	return false;
}

bool Ctab::ActiveTab(TabWindow* tab)
{
	auto it = std::find_if(tabs_.begin(), tabs_.end(),
		[tab](wunique_ptr<TabWindow>& el)
		{
			return (el.get() == tab);
		}
	);
	if (it == tabs_.end()) return false;

	auto activeone = it->get();
	int position = activeone->position_;
	if (current_ != position)
	{
		activeone->SetActivate(hover_activate);
		tab_->SetActivate(hover_inactivate);
		tab_ = activeone;
		current_ = position;
	}

	return true;
}

bool Ctab::SwapTab(int active, int inactive)
{
	if (active < 0 || inactive < 0)
	{
		return false;
	}

	int size = (int)tabs_.size();
	if (active >= size || inactive >= size)
	{
		return false;
	}

	auto x = std::next(tabs_.begin(), active);
	auto y = std::next(tabs_.begin(), inactive);

	x->get()->position_ = inactive;
	y->get()->position_ = active;
	y->get()->Reposition();

	x->swap(*y);
	current_ = inactive;

	return true;
}
