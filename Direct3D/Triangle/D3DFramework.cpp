#include "D3DFramework.h"

#pragma comment(lib,"d3d11.lib")

void D3DFramework::InitWinodw(HINSTANCE hInstance)
{
	mInstance = hInstance;

	WNDCLASSEX wc;
	ZeroMemory(&wc, sizeof(WNDCLASSEX));
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpszClassName = gClassName.c_str();
	wc.hInstance = mInstance;
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = 0;
	wc.lpfnWndProc = WindowProc;
	wc.cbSize = sizeof(WNDCLASSEX);

	if (!RegisterClassEx(&wc))
	{
		MessageBox(nullptr, L"Failed To Regiter WindowClass!!", L"Error", MB_OK);
		return;
	}

	RECT wr{ 0,0,mWindowWidth,mWindowHeight };
	AdjustWindowRect(&wr, WS_OVERLAPPEDWINDOW, FALSE);
	mHwnd = CreateWindowEx(
		NULL,
		gClassName.c_str(),
		gTitle.c_str(),
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT,
		wr.right - wr.left, wr.bottom - wr.top,
		NULL,
		NULL,
		mInstance,
		NULL
	);

	if (mHwnd == nullptr)
	{
		MessageBox(nullptr, L"Failed to Create Widnow!", L"Error", MB_OK);
		return;
	}

	SetWindowLongPtr(
		mHwnd, GWLP_USERDATA,
		reinterpret_cast<LONG_PTR>(this)
	);

	ShowWindow(mHwnd, SW_SHOW);
	SetForegroundWindow(mHwnd);
	SetFocus(mHwnd);
	UpdateWindow(mHwnd);
}

void D3DFramework::InitD3D()
{
	DXGI_SWAP_CHAIN_DESC scd{};

	ZeroMemory(&scd, sizeof(DXGI_SWAP_CHAIN_DESC));
	scd.BufferCount = 1;
	scd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	scd.BufferDesc.Width = mWindowWidth;
	scd.BufferDesc.Height = mWindowHeight;
	scd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	scd.OutputWindow = mHwnd;
	scd.SampleDesc.Count = 1;
	scd.Windowed = TRUE;
	scd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

	D3D11CreateDeviceAndSwapChain(
		NULL,
		D3D_DRIVER_TYPE_HARDWARE,
		NULL,
		0,
		NULL,
		0,
		D3D11_SDK_VERSION,
		&scd,
		mspSwapChanin.ReleaseAndGetAddressOf(),
		mspDevice.ReleaseAndGetAddressOf(),
		NULL,
		mspDeviceContext.ReleaseAndGetAddressOf()
	);

	OnResize();
}

void D3DFramework::OnResize()
{
	if (mspDevice == nullptr)
	{
		return;
	}

	ID3D11RenderTargetView* views[]{ nullptr };
	mspDeviceContext->OMSetRenderTargets(_countof(views), views, nullptr);

	mspRenderTargetView.Reset();
	mspRenderTarget.Reset();
	mspDepthStecilView.Reset();
	mspDepthStencil.Reset();

	mspDeviceContext->Flush();
	mspSwapChanin->ResizeBuffers(
		0,
		mWindowWidth, mWindowHeight,
		DXGI_FORMAT_UNKNOWN,
		0
	);

	mspSwapChanin->GetBuffer(0, IID_PPV_ARGS(mspRenderTarget.ReleaseAndGetAddressOf()));
	mspDevice->CreateRenderTargetView(
		mspRenderTarget.Get(),
		nullptr,
		mspRenderTargetView.ReleaseAndGetAddressOf());

	CD3D11_TEXTURE2D_DESC td(
		DXGI_FORMAT_D24_UNORM_S8_UINT,
		mWindowWidth,
		mWindowHeight,
		1,
		1,
		D3D11_BIND_DEPTH_STENCIL
	);

	mspDevice->CreateTexture2D(
		&td,
		nullptr,
		mspDepthStencil.ReleaseAndGetAddressOf());

	CD3D11_DEPTH_STENCIL_VIEW_DESC dsvd(
		D3D11_DSV_DIMENSION_TEXTURE2D);
	mspDevice->CreateDepthStencilView(
		mspDepthStencil.Get(),
		&dsvd,
		mspDepthStecilView.ReleaseAndGetAddressOf()
	);

	mspDeviceContext->OMSetRenderTargets(1,mspRenderTargetView.GetAddressOf(),mspDepthStecilView.Get());
}

void D3DFramework::DestroyD3D()
{
	mspSwapChanin->SetFullscreenState(FALSE, nullptr);

	mspDepthStecilView.Reset();
	mspDepthStencil.Reset();
	mspRenderTargetView.Reset();
	mspRenderTarget.Reset();
	mspSwapChanin.Reset();
	mspDevice.Reset();
	mspDeviceContext.Reset();
}

void D3DFramework::RenderFrame()
{
	CD3D11_VIEWPORT viewport(0.0f, 0.0f,
		static_cast<float>(mWindowWidth), static_cast<float>(mWindowHeight));
	mspDeviceContext->RSSetViewports(1, &viewport);

	const static float clear_color[4]{ 0.0f,0.2f,0.4f,1.0f };
	mspDeviceContext->ClearRenderTargetView(mspRenderTargetView.Get(), clear_color);
	mspDeviceContext->ClearDepthStencilView(mspDepthStecilView.Get(),
		D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL,
		1.0f,
		0);

	Render();

	mspSwapChanin->Present(0, 0);
}

void D3DFramework::Initizlize(HINSTANCE hInstance, int widht, int height)
{
	mWindowWidth = widht;
	mWindowHeight = height;
	InitWinodw(hInstance);
	InitD3D();
}

void D3DFramework::Destory()
{
	DestroyD3D();

	DestroyWindow(mHwnd);
	UnregisterClass(gClassName.c_str(), mInstance);
}

void D3DFramework::GameLoop()
{
	MSG msg{};

	while (true)
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);

			if (msg.message == WM_QUIT)
			{
				break;
			}
		}
		else
		{
			RenderFrame();
		}
	}
}

LRESULT D3DFramework::MessageHandler(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_ENTERSIZEMOVE:
		gResizing = true;
		break;

	case WM_SIZE:
		mWindowWidth = LOWORD(lParam);
		mWindowHeight = HIWORD(lParam);

		if (wParam == SIZE_MINIMIZED)
		{
			mMinimized = true;
			mMaximized = false;
		}
		else if (wParam == SIZE_MAXIMIZED)
		{
			mMinimized = false;
			mMaximized = true;
			OnResize();
		}
		else if (wParam == SIZE_RESTORED)
		{
			if (mMinimized)
			{
				mMinimized = false;
				OnResize();
			}
			else if (mMaximized)
			{
				mMaximized = false;
				OnResize();
			}
			else if (gResizing)
			{
				RenderFrame();
			}
			else
			{
				OnResize();
			}
		}
		break;

	case WM_EXITSIZEMOVE:
		gResizing = false;
		OnResize();
		break;

	case WM_GETMINMAXINFO:
		((MINMAXINFO*)lParam)->ptMinTrackSize.x = 640;
		((MINMAXINFO*)lParam)->ptMaxPosition.y = 480;
		break;

	case WM_MENUCHAR:
		return MAKELRESULT(0, MNC_CLOSE);
		break;

	case WM_CLOSE:
		DestroyWindow(hwnd);
		break;

	case WM_DESTROY:
		PostQuitMessage(0);
		break;

	default:
		return DefWindowProc(hwnd, message, wParam, lParam);
		break;
	}
	return 0;
}

LRESULT D3DFramework::WindowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	D3DFramework* pFramework = reinterpret_cast<D3DFramework*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));

	if (pFramework == nullptr)
	{
		return DefWindowProc(hwnd, message, wParam, lParam);
	}
	else
	{
		return pFramework->MessageHandler(hwnd, message, wParam, lParam);
	}
}
	
