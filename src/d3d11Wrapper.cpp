#include "../include/d3d11Wrapper.h"
#include "../include/geometry.h"

#include <d3dcompiler.h>

#include<assert.h>
#include<iostream>

#include "glm/ext.hpp"
#include <glm/gtc/matrix_access.hpp>

struct Uniform
{
	glm::mat4 model_world;
	glm::mat4 world_camera;
	glm::mat4 camera_ndc;
};

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
			d3dInfoQueue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_WARNING, true);
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

    D3D11_TEXTURE2D_DESC depthBufferDesc;
    d3d11FrameBuffer->GetDesc(&depthBufferDesc);
    depthBufferDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    depthBufferDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;

    ID3D11Texture2D* depthBuffer;
    d3d11Device->CreateTexture2D(&depthBufferDesc, nullptr, &depthBuffer);

    d3d11Device->CreateDepthStencilView(depthBuffer, nullptr, &d3d11DepthStencilView);

    d3d11FrameBuffer->Release();
    depthBuffer->Release();

}

// compile and create vertex + pixel shaders
void D3D11Wrapper::_d3d11_create_shaders(std::wstring vs_path, std::wstring ps_path)
{
	// Create Vertex Shader
	ID3DBlob *vsBlob;
	{
		ID3DBlob *shaderCompileErrorsBlob;
		HRESULT hResult = D3DCompileFromFile(vs_path.c_str(), nullptr, nullptr, "vs_main", "vs_5_0", D3DCOMPILE_DEBUG, 0, &vsBlob, &shaderCompileErrorsBlob);
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
		HRESULT hResult = D3DCompileFromFile(ps_path.c_str(), nullptr, nullptr, "ps_main", "ps_5_0", D3DCOMPILE_DEBUG, 0, &psBlob, &shaderCompileErrorsBlob);
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
				{"POS", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
				{"TEX", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
			};

		HRESULT hResult = d3d11Device->CreateInputLayout(inputElementDesc, ARRAYSIZE(inputElementDesc), vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), &inputLayout);
		assert(SUCCEEDED(hResult));
		vsBlob->Release();
	}

	// Create Vertex and Index Buffer
	{
		std::vector<Vertex_attribute> verts {};
		for(auto& v: state->m_model_original.m_gpu.verts)
		{
			verts.push_back(v);
		}

		D3D11_BUFFER_DESC vertexBufferDesc = {};
		auto vs =  sizeof(Vertex_attribute);
		vertexBufferDesc.ByteWidth = verts.size() * vs;
		vertexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
		vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

		D3D11_SUBRESOURCE_DATA vertexSubresourceData = {verts.data()};

		HRESULT hResult = d3d11Device->CreateBuffer(&vertexBufferDesc, &vertexSubresourceData, &vertexBuffer);
		assert(SUCCEEDED(hResult));

		// std::vector<uint16_t> p_indecies {};
		// for (auto &face : state->m_model_original.m_cpu.faces)
		// {
		// 	p_indecies.push_back(face.p_indices.x);
		// 	p_indecies.push_back(face.p_indices.y);
		// 	p_indecies.push_back(face.p_indices.z);
		// }

		// D3D11_BUFFER_DESC indexBufferDesc = {};
		// indexBufferDesc.ByteWidth = p_indecies.size() * sizeof(uint16_t);
		// indexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
		// indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;

		// D3D11_SUBRESOURCE_DATA indexSubresourceData = {p_indecies.data()};

		// hResult = d3d11Device->CreateBuffer(&indexBufferDesc, &indexSubresourceData, &indexBuffer);
		// assert(SUCCEEDED(hResult));
	}
}

void D3D11Wrapper::_d3d11_create_sampler_state() 
{
	// Create Sampler State
	D3D11_SAMPLER_DESC samplerDesc = {};
    samplerDesc.Filter         = D3D11_FILTER_MIN_MAG_MIP_POINT;
    samplerDesc.AddressU       = D3D11_TEXTURE_ADDRESS_BORDER;
    samplerDesc.AddressV       = D3D11_TEXTURE_ADDRESS_BORDER;
    samplerDesc.AddressW       = D3D11_TEXTURE_ADDRESS_BORDER;
    samplerDesc.BorderColor[0] = 1.0f;
    samplerDesc.BorderColor[1] = 1.0f;
    samplerDesc.BorderColor[2] = 1.0f;
    samplerDesc.BorderColor[3] = 1.0f;
    samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;

	d3d11Device->CreateSamplerState(&samplerDesc, &samplerState);
}

void D3D11Wrapper::_d3d11_create_texture(Texture t)
{
	// Create Texture
	D3D11_TEXTURE2D_DESC textureDesc = {};
	textureDesc.Width = t.width;
	textureDesc.Height = t.height;
	textureDesc.MipLevels = 1;
	textureDesc.ArraySize = 1;
	textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.Usage = D3D11_USAGE_IMMUTABLE;
	textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

	D3D11_SUBRESOURCE_DATA textureSubresourceData = {};
	textureSubresourceData.pSysMem = t.data;
	textureSubresourceData.SysMemPitch = t.bytes_per_pixel * t.width;

	d3d11Device->CreateTexture2D(&textureDesc, &textureSubresourceData, &texture);
	d3d11Device->CreateShaderResourceView(texture, nullptr, &textureView);
}

ID3D11Buffer* D3D11Wrapper::_d3d11_create_cbuffer(uint32_t size)
{
	// TODO[adel]: handle resources better, (i.e. push the resources handles to a vector, and release them on destruction...)
	ID3D11Buffer *constantBuffer;
	D3D11_BUFFER_DESC constantBufferDesc = {};
	// ByteWidth must be a multiple of 16, per the docs
	constantBufferDesc.ByteWidth = size + 0xf & 0xfffffff0;
	constantBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	constantBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	constantBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

	HRESULT hResult = d3d11Device->CreateBuffer(&constantBufferDesc, nullptr, &constantBuffer);
	assert(SUCCEEDED(hResult));
	return constantBuffer;
}

void D3D11Wrapper::_d3d11_update_cbuffer(ID3D11Buffer *cbuffer, void *data, uint32_t size)
{
	// TODO[adel]: assert the cbuffer size eqauls the data size
	D3D11_MAPPED_SUBRESOURCE mappedSubresource;
	d3d11DeviceContext->Map(cbuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedSubresource);
	memcpy(mappedSubresource.pData, data, size);
	d3d11DeviceContext->Unmap(cbuffer, 0);
}

void D3D11Wrapper::_d3d11_create_rasterizer_state()
{
	D3D11_RASTERIZER_DESC rasterizerDesc = {};
	rasterizerDesc.FillMode = D3D11_FILL_SOLID;
	rasterizerDesc.CullMode = D3D11_CULL_NONE;
	rasterizerDesc.FrontCounterClockwise = TRUE;

	d3d11Device->CreateRasterizerState(&rasterizerDesc, &rasterizerState);
}

void D3D11Wrapper::_d3d11_create_depth_stencil_state()
{
	D3D11_DEPTH_STENCIL_DESC depthStencilDesc = {};
	depthStencilDesc.DepthEnable = TRUE;
	depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	depthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS;

	d3d11Device->CreateDepthStencilState(&depthStencilDesc, &depthStencilState);
}

void D3D11Wrapper::initD3D11()
{
	_d3d11_create_device();
	if(enableDebugLayer)
		_d3d11_set_debug_layer();
	_d3d11_create_swapchain();
	_d3d11_create_render_target();
	_d3d11_create_shaders(L"../../assets/shaders/shaders.hlsl",L"../../assets/shaders/shaders.hlsl");
	_d3d11_create_rasterizer_state();
	_d3d11_create_depth_stencil_state();
	_d3d11_create_sampler_state();
	_d3d11_create_texture(state->m_model.m_gpu.textures[0]);
	cbuffer = _d3d11_create_cbuffer(sizeof(Uniform));

}

void D3D11Wrapper::render_frame()
{
	FLOAT backgroundColor[4] = {0.1f, 0.2f, 0.6f, 1.0f};
	backgroundColor[0] = backgroundColor[0] >= 1.0f ? 0.0f : backgroundColor[0] + .01f;
	d3d11DeviceContext->ClearRenderTargetView(d3d11FrameBufferView, backgroundColor);
	d3d11DeviceContext->ClearDepthStencilView(d3d11DepthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);

	d3d11DeviceContext->RSSetState(rasterizerState);
	d3d11DeviceContext->OMSetDepthStencilState(depthStencilState, 0);

	RECT winRect;
	GetClientRect(state->m_window.win32_win, &winRect);
	D3D11_VIEWPORT viewport = {0.0f, 0.0f, (FLOAT)(winRect.right - winRect.left), (FLOAT)(winRect.bottom - winRect.top), 0.0f, 1.0f};
	d3d11DeviceContext->RSSetViewports(1, &viewport);

	d3d11DeviceContext->OMSetRenderTargets(1, &d3d11FrameBufferView, d3d11DepthStencilView);

	d3d11DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	d3d11DeviceContext->IASetInputLayout(inputLayout);

	d3d11DeviceContext->VSSetShader(vertexShader, nullptr, 0);
	d3d11DeviceContext->PSSetShader(pixelShader, nullptr, 0);

	d3d11DeviceContext->PSSetShaderResources(0, 1, &textureView);
	d3d11DeviceContext->PSSetSamplers(0, 1, &samplerState);

	Geometry gm;
	gm.bind_state(state);
	static bool once = true;
	gm.update_world_transform();
	gm.update_camera_transform();
	gm.update_perspective_transform();

	// auto modelViewProj =
	// 	gm.camera_ndc_transform *
	// 	gm.world_camera_transform *
	// 	gm.model_world_transform;

	// modelViewProj = glm::transpose(modelViewProj);
	// _d3d11_update_cbuffer(cbuffer, glm::value_ptr(modelViewProj), sizeof(glm::mat4));

	Uniform u{};
	u.model_world = gm.model_world_transform;
	u.world_camera = gm.world_camera_transform;
	u.camera_ndc = gm.camera_ndc_transform;

	_d3d11_update_cbuffer(cbuffer, &u, sizeof(Uniform));
    d3d11DeviceContext->VSSetConstantBuffers(0, 1, &cbuffer);

	UINT stride = sizeof(Vertex_attribute);
	UINT offset = 0;
	d3d11DeviceContext->IASetVertexBuffers(0, 1, &vertexBuffer, &stride, &offset);
	// d3d11DeviceContext->IASetIndexBuffer(indexBuffer, DXGI_FORMAT_R16_UINT, 0);

	// d3d11DeviceContext->Draw(state->m_model.m_gpu.faces.size() * 3, 0, 0);
	d3d11DeviceContext->Draw(state->m_model.m_gpu.verts.size(), 0);

	d3d11SwapChain->Present(1, 0);
}

void D3D11Wrapper::cleanup()
{
	std::cout << "Cleanup...\n";
	// indexBuffer->Release();
	vertexBuffer->Release();
	inputLayout->Release();
	vertexShader->Release();
	pixelShader->Release();
	d3d11FrameBufferView->Release();
	d3d11SwapChain->Release();
	d3d11DeviceContext->Release();
	d3d11Device->Release();
}