#include "../include/d3d11Wrapper.h"

#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

#include<assert.h>
#include<iostream>

void D3D11Wrapper::run()
{
	initWindow();
	initD3D11();
	mainLoop();
	cleanup();
}

// Create Device and Context
void D3D11Wrapper::_d3d11_create_device()
{
	ID3D11Device *baseDevice;
	ID3D11DeviceContext *baseDeviceContext;
	D3D_FEATURE_LEVEL featureLevels[] = {D3D_FEATURE_LEVEL_11_0};
	UINT creationFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;
#ifndef NDEBUG
	creationFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

	HRESULT hResult = D3D11CreateDevice(0, D3D_DRIVER_TYPE_HARDWARE,
										0, creationFlags,
										featureLevels, ARRAYSIZE(featureLevels),
										D3D11_SDK_VERSION, &baseDevice,
										0, &baseDeviceContext);
	if (FAILED(hResult))
	{
		std::cerr << "D3D11CreateDevice() failed\n"
				  << GetLastError() << std::endl;
		return;
	}

	// Get 1.1 interface of D3D11 Device and Context
	hResult = baseDevice->QueryInterface(__uuidof(ID3D11Device1), (void **)&d3d11Device);
	assert(SUCCEEDED(hResult));
	baseDevice->Release();

	hResult = baseDeviceContext->QueryInterface(__uuidof(ID3D11DeviceContext1), (void **)&d3d11DeviceContext);
	assert(SUCCEEDED(hResult));
	baseDeviceContext->Release();
}

// Set up debug layer to break on D3D11 errors
void D3D11Wrapper::_d3d11_set_debug_layer()
{
	ID3D11Debug *d3dDebug = nullptr;
	d3d11Device->QueryInterface(__uuidof(ID3D11Debug), (void **)&d3dDebug);
	if (d3dDebug)
	{
		ID3D11InfoQueue *d3dInfoQueue = nullptr;
		if (SUCCEEDED(d3dDebug->QueryInterface(__uuidof(ID3D11InfoQueue), (void **)&d3dInfoQueue)))
		{
			d3dInfoQueue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_CORRUPTION, true);
			d3dInfoQueue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_ERROR, true);
			d3dInfoQueue->Release();
		}
		d3dDebug->Release();
	}
}

// Create Swap Chain
void D3D11Wrapper::_d3d11_create_swapchain()
{
	// Get DXGI Factory (needed to create Swap Chain)
	IDXGIFactory2 *dxgiFactory;
	{
		IDXGIDevice1 *dxgiDevice;
		HRESULT hResult = d3d11Device->QueryInterface(__uuidof(IDXGIDevice1), (void **)&dxgiDevice);
		assert(SUCCEEDED(hResult));

		IDXGIAdapter *dxgiAdapter;
		hResult = dxgiDevice->GetAdapter(&dxgiAdapter);
		assert(SUCCEEDED(hResult));
		dxgiDevice->Release();

		DXGI_ADAPTER_DESC adapterDesc;
		dxgiAdapter->GetDesc(&adapterDesc);

		std::wcout << "Graphics Device: " << adapterDesc.Description << std::endl;

		hResult = dxgiAdapter->GetParent(__uuidof(IDXGIFactory2), (void **)&dxgiFactory);
		assert(SUCCEEDED(hResult));
		dxgiAdapter->Release();
	}

	DXGI_SWAP_CHAIN_DESC1 d3d11SwapChainDesc = {};
	d3d11SwapChainDesc.Width = 0;  // use window width
	d3d11SwapChainDesc.Height = 0; // use window height
	d3d11SwapChainDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM_SRGB;
	d3d11SwapChainDesc.SampleDesc.Count = 1;
	d3d11SwapChainDesc.SampleDesc.Quality = 0;
	d3d11SwapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	d3d11SwapChainDesc.BufferCount = 2;
	d3d11SwapChainDesc.Scaling = DXGI_SCALING_STRETCH;
	d3d11SwapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	d3d11SwapChainDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
	d3d11SwapChainDesc.Flags = 0;

	HRESULT hResult = dxgiFactory->CreateSwapChainForHwnd(d3d11Device,
														  glfwGetWin32Window(window),
														  &d3d11SwapChainDesc,
														  0, 0, &d3d11SwapChain);
	assert(SUCCEEDED(hResult));

	dxgiFactory->Release();
}

// Create Framebuffer Render Target
void D3D11Wrapper::_d3d11_create_render_target()
{
	ID3D11Texture2D *d3d11FrameBuffer;
	HRESULT hResult = d3d11SwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void **)&d3d11FrameBuffer);
	assert(SUCCEEDED(hResult));

	hResult = d3d11Device->CreateRenderTargetView(d3d11FrameBuffer, 0, &d3d11FrameBufferView);
	assert(SUCCEEDED(hResult));
	d3d11FrameBuffer->Release();
}

void D3D11Wrapper::initD3D11()
{
	_d3d11_create_device();
	if(enableDebugLayer)
		_d3d11_set_debug_layer();
	_d3d11_create_swapchain();
	_d3d11_create_render_target();
}

void D3D11Wrapper::initWindow()
{
	glfwInit();
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

	window = glfwCreateWindow(win_width, win_height, "Learning D3D11!", nullptr, nullptr);
	assert(window);
	if(window == NULL)
	{
		std::cerr << "Failed to create a window\n";
	}
}

void D3D11Wrapper::mainLoop()
{
	FLOAT backgroundColor[4] = {0.1f, 0.2f, 0.6f, 1.0f};
	while (!glfwWindowShouldClose(window))
	{
		glfwPollEvents();

		backgroundColor[0] = backgroundColor[0] >= 1.0f ? 0.0f : backgroundColor[0] + .01f;
		d3d11DeviceContext->ClearRenderTargetView(d3d11FrameBufferView, backgroundColor);

		d3d11SwapChain->Present(1, 0);
	}
}

void D3D11Wrapper::cleanup()
{
	std::cout << "Cleanup...\n";
	d3d11FrameBufferView->Release();
	d3d11SwapChain->Release();
	d3d11DeviceContext->Release();
	d3d11Device->Release();
	glfwDestroyWindow(window);
}