#include <pch.h>
#include <Win32Windows/HostDIMG.h>

#include <imgui.h>
#include <imgui_impl_win32.h>
#include <imgui_impl_dx11.h>
#include <d3d11.h>

using namespace NativeWindows;

HostDIMG::HostDIMG()
{
	thread_ = this;
	OnThreadStarts = [](Win32UIThread* sender)
	{
		HostDIMG* inst = (HostDIMG*)sender;
		inst->CreateEx();
	};
}
HostDIMG::~HostDIMG()
{
	Close();
}

LRESULT HostDIMG::OnCreate1(LPCREATESTRUCT createstr)
{
	CreateSwapchain();
	return 0;
}

LRESULT HostDIMG::OnSize(WPARAM state, int width, int height)
{
	ResizeSwapchain(width, height);
	return 0;
}

void HostDIMG::CreateSwapchain()
{
	visualf_ = refres_->compositor_.CreateSpriteVisual();
	visualf_.RelativeSizeAdjustment({ 1.0f, 1.0f });

	auto brush = refres_->compositor_.CreateSurfaceBrush();
	brush.Stretch(CompositionStretch::None);

	visualf_.Brush(brush);
	visuals_.InsertAtTop(visualf_);

	DXGI_SWAP_CHAIN_DESC1 swapChainDesc{ 0 };
	swapChainDesc.Width = rect_.width;
	swapChainDesc.Height = rect_.height;
	swapChainDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
	swapChainDesc.Stereo = false;
	swapChainDesc.SampleDesc.Count = 1;
	swapChainDesc.SampleDesc.Quality = 0;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.BufferCount = 2;
	swapChainDesc.Scaling = DXGI_SCALING_STRETCH;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;
	swapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

	HR(refres_->dxgiFactory_->CreateSwapChainForComposition(
		refres_->d3dDevice_.get(),
		&swapChainDesc,
		nullptr,
		swapchain_.put()));

	ID3D11Texture2D* backbuffer = nullptr;
	HR(swapchain_->GetBuffer(0, __uuidof(backbuffer), (void**)&backbuffer));

	surfacef_ = CreateCompositionSurfaceForSwapChain(refres_->compositor_, swapchain_.get());
	brush.Surface(surfacef_);

	HR(refres_->d3dDevice_->CreateRenderTargetView(
		backbuffer , nullptr, &renderTargetView_));
	backbuffer->Release();
}

void HostDIMG::ResizeSwapchain(int w, int h)
{
	SafeRelease(&renderTargetView_);
	swapchain_->ResizeBuffers(0, w, h, DXGI_FORMAT_UNKNOWN, 0);
	
	ID3D11Texture2D* backbuffer = nullptr;
	HR(swapchain_->GetBuffer(0, __uuidof(backbuffer), (void**)&backbuffer));
	surfacef_ = CreateCompositionSurfaceForSwapChain(refres_->compositor_, swapchain_.get());
	HR(swapchain_->Present(0, 0));

	HR(refres_->d3dDevice_->CreateRenderTargetView(
		backbuffer, nullptr, &renderTargetView_));
	backbuffer->Release();
}


void HostDIMG::handlemsg()
{
	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

	// Setup Dear ImGui style
	ImGui::StyleColorsDark();
	//ImGui::StyleColorsLight();

	// Setup Platform/Renderer backends
	ImGui_ImplWin32_Init(hwnd_);
	ImGui_ImplDX11_Init(refres_->d3dDevice_.get(), refres_->d3dDc_.get());

	bool show_demo_window = true;
	bool show_another_window = false;
	ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

	while (!bDone_)
	{
		// Poll and handle messages (inputs, window resize, etc.)
		// See the WndProc() function below for our to dispatch events to the Win32 backend.
		MSG Msg;
		if (!bShow_)
		{
			while (::GetMessage(&Msg, nullptr, 0, 0))
			{
				handlemsg_(Msg);
				if (bDone_ || bShow_)
					break;
			}
		}
		else
		{
			while (::PeekMessage(&Msg, NULL, 0U, 0U, PM_REMOVE))
			{
				handlemsg_(Msg);
			}
		}

		if (bDone_)
		{
			break;
		}

		// Start the Dear ImGui frame
		ImGui_ImplDX11_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();

		if (show_demo_window)
			ImGui::ShowDemoWindow(&show_demo_window);

		{
			static float f = 0.0f;
			static int counter = 0;

			ImGui::Begin("Hello, world!");                          // Create a window called "Hello, world!" and append into it.

			ImGui::Text("This is some useful text.");               // Display some text (you can use a format strings too)
			ImGui::Checkbox("Demo Window", &show_demo_window);      // Edit bools storing our window open/close state
			ImGui::Checkbox("Another Window", &show_another_window);

			ImGui::SliderFloat("float", &f, 0.0f, 1.0f);            // Edit 1 float using a slider from 0.0f to 1.0f
			ImGui::ColorEdit3("clear color", (float*)&clear_color); // Edit 3 floats representing a color

			if (ImGui::Button("Button"))                            // Buttons return true when clicked (most widgets return true when edited/activated)
				counter++;
			ImGui::SameLine();
			ImGui::Text("counter = %d", counter);

			ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
			ImGui::End();
		}

		if (show_another_window)
		{
			ImGui::Begin("Another Window", &show_another_window);   // Pass a pointer to our bool variable (the window will have a closing button that will clear the bool when clicked)
			ImGui::Text("Hello from another window!");
			if (ImGui::Button("Close Me"))
				show_another_window = false;
			ImGui::End();
		}

		// Rendering
		ImGui::Render();
		const float clear_color_with_alpha[4] = { clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w };
		refres_->d3dDc_->OMSetRenderTargets(1, &renderTargetView_, NULL);
		refres_->d3dDc_->ClearRenderTargetView(renderTargetView_, clear_color_with_alpha);
		ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

		swapchain_->Present(1, 0); // Present with vsync
	}

	ImGui_ImplDX11_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();

	SafeRelease(&renderTargetView_);
}

void HostDIMG::handlemsg_(MSG Msg)
{
	switch (Msg.message)
	{
	case UTM_QUIT:
	{
		bDone_ = true;
		::DestroyWindow(hmsgw_);
		::PostQuitMessage(0);
		break;
	}
	default:
	{
		TranslateMessage(&Msg);
		DispatchMessage(&Msg);
		break;
	}
	}
}

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT HostDIMG::WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam))
	{
		return true;
	}
	else
	{
		switch (uMsg)
		{
		case WM_SHOWWINDOW:
		{
			if (wParam)
			{
				bShow_ = true;
			}
			else
			{
				bShow_ = false;
			}
			return 0;
		}
		case WM_SYSCOMMAND:
		{
			if ((wParam & 0xfff0) == SC_KEYMENU) // Disable ALT application menu
				break;
			return 0;
		}
		default:
			break;
		}
		return Win32Window::WndProc(hWnd, uMsg, wParam, lParam);
	}
}
