#include "../include/d3d11Wrapper.h"

#include <d3dcompiler.h>

#include<assert.h>
#include<iostream>

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
														  state->m_window.win32_win,
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

// compile and create vertex + pixel shaders
void D3D11Wrapper::_d3d11_create_shaders(std::wstring vs_path, std::wstring ps_path)
{
	// Create Vertex Shader
	ID3DBlob *vsBlob;
	{
		ID3DBlob *shaderCompileErrorsBlob;
		HRESULT hResult = D3DCompileFromFile(vs_path.c_str(), nullptr, nullptr, "vs_main", "vs_5_0", 0, 0, &vsBlob, &shaderCompileErrorsBlob);
		if (FAILED(hResult))
		{
			const char *errorString = NULL;
			if (hResult == HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND))
				errorString = "Could not compile shader; file not found";
			else if (shaderCompileErrorsBlob)
			{
				errorString = (const char *)shaderCompileErrorsBlob->GetBufferPointer();
				shaderCompileErrorsBlob->Release();
			}
			std::cerr << "Shader Compiler Error: " << errorString << std::endl;
			return;
		}

		hResult = d3d11Device->CreateVertexShader(vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), nullptr, &vertexShader);
		assert(SUCCEEDED(hResult));
	}

	// Create Pixel Shader
	{
		ID3DBlob *psBlob;
		ID3DBlob *shaderCompileErrorsBlob;
		HRESULT hResult = D3DCompileFromFile(ps_path.c_str(), nullptr, nullptr, "ps_main", "ps_5_0", 0, 0, &psBlob, &shaderCompileErrorsBlob);
		if (FAILED(hResult))
		{
			const char *errorString = NULL;
			if (hResult == HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND))
				errorString = "Could not compile shader; file not found";
			else if (shaderCompileErrorsBlob)
			{
				errorString = (const char *)shaderCompileErrorsBlob->GetBufferPointer();
				shaderCompileErrorsBlob->Release();
			}
			std::cerr << "Shader Compiler Error: " << errorString << std::endl;
			return;
		}

		hResult = d3d11Device->CreatePixelShader(psBlob->GetBufferPointer(), psBlob->GetBufferSize(), nullptr, &pixelShader);
		assert(SUCCEEDED(hResult));
		psBlob->Release();
	}

	// Create Input Layout
	{
		D3D11_INPUT_ELEMENT_DESC inputElementDesc[] =
			{
				{"POS", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
				{"COL", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0}};

		HRESULT hResult = d3d11Device->CreateInputLayout(inputElementDesc, ARRAYSIZE(inputElementDesc), vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), &inputLayout);
		assert(SUCCEEDED(hResult));
		vsBlob->Release();
	}

	// Create Vertex Buffer
	UINT numVerts;
	UINT stride;
	UINT offset;
	{
		float vertexData[] = {// x, y, r, g, b, a
							  0.0f, 0.5f, 0.f, 1.f, 0.f, 1.f,
							  0.5f, -0.5f, 1.f, 0.f, 0.f, 1.f,
							  -0.5f, -0.5f, 0.f, 0.f, 1.f, 1.f};
		stride = 6 * sizeof(float);
		numVerts = sizeof(vertexData) / stride;
		offset = 0;

		D3D11_BUFFER_DESC vertexBufferDesc = {};
		vertexBufferDesc.ByteWidth = sizeof(vertexData);
		vertexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
		vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

		D3D11_SUBRESOURCE_DATA vertexSubresourceData = {vertexData};

		HRESULT hResult = d3d11Device->CreateBuffer(&vertexBufferDesc, &vertexSubresourceData, &vertexBuffer);
		assert(SUCCEEDED(hResult));
	}
}

void D3D11Wrapper::initD3D11()
{
	_d3d11_create_device();
	if(enableDebugLayer)
		_d3d11_set_debug_layer();
	_d3d11_create_swapchain();
	_d3d11_create_render_target();
	_d3d11_create_shaders(L"../../assets/shaders/shaders.hlsl",L"../../assets/shaders/shaders.hlsl");
}

void D3D11Wrapper::render_frame()
{
	FLOAT backgroundColor[4] = {0.1f, 0.2f, 0.6f, 1.0f};

	backgroundColor[0] = backgroundColor[0] >= 1.0f ? 0.0f : backgroundColor[0] + .01f;
	d3d11DeviceContext->ClearRenderTargetView(d3d11FrameBufferView, backgroundColor);

	RECT winRect;
	GetClientRect(state->m_window.win32_win, &winRect);
	D3D11_VIEWPORT viewport = {0.0f, 0.0f, (FLOAT)(winRect.right - winRect.left), (FLOAT)(winRect.bottom - winRect.top), 0.0f, 1.0f};
	d3d11DeviceContext->RSSetViewports(1, &viewport);

	d3d11DeviceContext->OMSetRenderTargets(1, &d3d11FrameBufferView, nullptr);

	d3d11DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	d3d11DeviceContext->IASetInputLayout(inputLayout);

	d3d11DeviceContext->VSSetShader(vertexShader, nullptr, 0);
	d3d11DeviceContext->PSSetShader(pixelShader, nullptr, 0);

	UINT stride = 6 * sizeof(float);
	UINT numVerts = 3;
	UINT offset = 0;
	d3d11DeviceContext->IASetVertexBuffers(0, 1, &vertexBuffer, &stride, &offset);

	d3d11DeviceContext->Draw(numVerts, 0);

	d3d11SwapChain->Present(1, 0);
}

void D3D11Wrapper::cleanup()
{
	std::cout << "Cleanup...\n";
	vertexBuffer->Release();
	inputLayout->Release();
	vertexShader->Release();
	pixelShader->Release();
	d3d11FrameBufferView->Release();
	d3d11SwapChain->Release();
	d3d11DeviceContext->Release();
	d3d11Device->Release();
}