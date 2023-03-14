#include <pch.h>
#include <Win32Windows/TabIMGUI.h>
#include <NativeWindows2/windows/ClientWindow.h>
#include <Win32Windows/HostDIMG.h>

using namespace NativeWindows;

TabIMGUI::TabIMGUI(Ctab* parent) : TabWindow(parent)
{
	title_ = L"DearIMGUI";

	OnSetup = [](TabWindow* sender)
	{
		TabIMGUI* tab = (TabIMGUI*)sender;
		HostDIMG* content = new HostDIMG();
		tab->SetContent(content);
	};

	OnTabClose = [](TabWindow* sender)
	{
	};
}

TabIMGUI::~TabIMGUI()
{

}

void TabIMGUI::SetClient()
{
	auto client = new ClientWindow1(this);
	client->OnShowWindow = [](ClientWindow* sender, int cmd)
	{
		sender->GetContent()->ShowWindowAsync(cmd);
	};
	client_ = wunique_ptr<ClientWindow>(client);	
}
