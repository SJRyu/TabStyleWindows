#include <pch.h>
#include <MyApp.h>
#include <NativeWindows2/windows/WindowProperties.h>
#include <NativeWindows2/windows/Win32Window.h>
#include <NativeWindows2/windows/ContainerWindow.h>
#include <NativeWindows2/windows/Ctab.h>
#include <NativeWindows2/windows/TabWindow.h>
#include <Win32Windows/NvrTab.h>
#include <Win32Windows/NvrWindow.h>
#include <NativeWindows2/windows/CButton.h>
#include <Win32Windows/TestTab.h>
#include <AssetManager.h>

using namespace NativeWindows;
using namespace winrt;

MyApp* MyApp::instance_ = nullptr;
HMENU MyApp::hmenu_ = nullptr;

MyApp::MyApp()
{
	AssetManager::InitAssets();
	hmenu_ = ::LoadMenu(GetModuleHandle(NULL), MAKEINTRESOURCE(IDR_MENU1));
}

MyApp::~MyApp()
{
	::DestroyMenu(hmenu_);
	AssetManager::ReleaseAssets();
	instance_ = nullptr;
}

void MyApp::OnAppStart()
{
	discoveryThread_ = new DiscoveryThread();
	discoveryThread_->Start();

	UINT dpi = GetDpiForSystem();
	LONG hx = GetSystemMetricsForDpi(SM_CXSCREEN, dpi) / 2;
	LONG hy = GetSystemMetricsForDpi(SM_CYSCREEN, dpi) / 2;

	RECT rect{ hx / 2, hy / 2 };
	rect.right = rect.left + hx;
	rect.bottom = rect.top + hy;

	auto win = NewContainer(rect);
	auto ctab = win->ctab_.get();

	NvrTab* ntab = new NvrTab(ctab);
	ctab->PostAdd((TabWindow*)ntab);
}

void MyApp::OnAppEnd()
{
	delete discoveryThread_;
}

void MyApp::OnNewContainer(ContainerWindow* win)
{
	auto btnadd = win->ctab_->btnadd_.get();
	btnadd->OnClick = [](CButton* btn)
	{
		Ctab* ctab = (Ctab*)btn->parent_;
		NvrTab* ntab = new NvrTab(ctab);
		ctab->PostAdd((TabWindow*)ntab);
	};
	btnadd->ShowWindow();
}
