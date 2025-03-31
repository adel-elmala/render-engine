#include "../include/d3d11Wrapper.h"
#include "../include/geometry.h"

#include <d3dcompiler.h>

#include <assert.h>
#include <iostream>

#include "glm/ext.hpp"
#include <glm/gtc/matrix_access.hpp>


struct Uniform_
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
			d3dInfoQueue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_WARNING, false);
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
std::tuple<ID3D11Texture2D *, ID3D11ShaderResourceView *, ID3D11RenderTargetView *, ID3D11Texture2D *, ID3D11DepthStencilView *> 
D3D11Wrapper::_d3d11_create_render_texture(size_t width, size_t height, size_t bytes_per_pixel)
{
	// Create Texture
	auto [texture, srv] = _d3d11_create_texture(width, height, TEXTURE_BIND_FLAGS_RENDER_TARGET, nullptr);

	ID3D11RenderTargetView* rtv{};
	auto hResult = d3d11Device->CreateRenderTargetView(texture, 0, &rtv);
	assert(SUCCEEDED(hResult));

	auto [depth, dsv] = _d3d11_create_depth_texture(width, height, bytes_per_pixel);
	return {texture, srv, rtv, depth, dsv};
}

// Create Framebuffer Render Target
void D3D11Wrapper::_d3d11_create_render_target()
{
	ID3D11Texture2D *texture;
	HRESULT hResult = d3d11SwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void **)&texture);
	assert(SUCCEEDED(hResult));

	hResult = d3d11Device->CreateRenderTargetView(texture, 0, &d3d11FrameBufferView);
	assert(SUCCEEDED(hResult));

	this->textures.push_back(texture);
	this->texture_views.push_back(d3d11FrameBufferView);
}

// compile and create vertex + pixel shaders
std::pair<ID3D11VertexShader*, ID3D11PixelShader*> D3D11Wrapper::_d3d11_create_overlay_shader(std::wstring path, std::string vs_entry, std::string ps_entry)
{
	auto vs = _d3d11_create_vertex_shader(path, vs_entry);
	auto ps = _d3d11_create_pixel_shader(path, ps_entry);
	return {vs, ps};
}

// compile and create vertex + pixel shaders
std::pair<ID3D11VertexShader*, ID3D11PixelShader*> D3D11Wrapper::_d3d11_create_env_map_shader(std::wstring path, std::string vs_entry, std::string ps_entry)
{
	auto vs = _d3d11_create_vertex_shader(path, vs_entry);
	auto ps = _d3d11_create_pixel_shader(path, ps_entry);
	return {vs, ps};
}

ID3D11VertexShader* D3D11Wrapper::_d3d11_create_vertex_shader(std::wstring path, std::string entry)
{
	// Create Vertex Shader
	ID3DBlob *vsBlob;
	ID3DBlob *shaderCompileErrorsBlob;
	ID3D11VertexShader* shader;

	HRESULT hResult = D3DCompileFromFile(path.c_str(), nullptr, nullptr, entry.c_str(), "vs_5_0", D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION, 1, &vsBlob, &shaderCompileErrorsBlob);
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
		return nullptr;
	}

	hResult = d3d11Device->CreateVertexShader(vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), nullptr, &shader);
	assert(SUCCEEDED(hResult));

	this->shaders.push_back(shader);
	this->compiled_vs_shaders[shader] = vsBlob;
	return shader;
}

ID3D11PixelShader* D3D11Wrapper::_d3d11_create_pixel_shader(std::wstring path, std::string entry)
{
	// Create Pixel Shader
	ID3DBlob *psBlob;
	ID3DBlob *shaderCompileErrorsBlob;
	ID3D11PixelShader* shader;

	HRESULT hResult = D3DCompileFromFile(path.c_str(), nullptr, nullptr, entry.c_str(), "ps_5_0", D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION, 0, &psBlob, &shaderCompileErrorsBlob);
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
		return nullptr;
	}

	hResult = d3d11Device->CreatePixelShader(psBlob->GetBufferPointer(), psBlob->GetBufferSize(), nullptr, &shader);
	assert(SUCCEEDED(hResult));
	this->shaders.push_back(shader);
	this->compiled_ps_shaders[shader] = psBlob;
	return shader;
}

const char *_d3d11_v_attribute_type_to_semantic_name(V_ATTRIBUTE_TYPE t)
{
	switch (t)
	{
	case V_ATTRIBUTE_TYPE_POSITION:
		return "POS";
	case V_ATTRIBUTE_TYPE_NORMAL:
		return "NORMAL";
	case V_ATTRIBUTE_TYPE_TEXTURE_COORD:
		return "TEX";
	}
}

DXGI_FORMAT _d3d11_engine_format_to_dxgi_format(FORMAT f)
{
	switch (f)
	{
	case FORMAT_R32G32B32A32_FLOAT:
		return DXGI_FORMAT_R32G32B32A32_FLOAT;
	case FORMAT_R32G32_FLOAT:
		return DXGI_FORMAT_R32G32_FLOAT;
	}
}
// Create Input Layout
ID3D11InputLayout *D3D11Wrapper::_d3d11_create_input_layout(Shader vs, Input_Layout layout)
{
	ID3D11InputLayout *d3d11_layout;
	std::vector<D3D11_INPUT_ELEMENT_DESC> inputElementDesc;

	size_t i = 0;
	for (auto &elmt_desc : layout.elements)
	{
		D3D11_INPUT_ELEMENT_DESC desc{};
		desc.AlignedByteOffset = i++ == 0 ? 0 : D3D11_APPEND_ALIGNED_ELEMENT;
		desc.InputSlot = 0;
		desc.InstanceDataStepRate = 0;
		desc.SemanticIndex = 0;
		desc.Format = _d3d11_engine_format_to_dxgi_format(elmt_desc.format);
		desc.InputSlotClass = elmt_desc.freq == V_ATTRIBUTE_FREQ_PER_VERTEX ? D3D11_INPUT_PER_VERTEX_DATA : D3D11_INPUT_PER_INSTANCE_DATA;
		desc.SemanticName = _d3d11_v_attribute_type_to_semantic_name(elmt_desc.type); 

		inputElementDesc.push_back(desc);
	}

	if (i == 0)
		return nullptr;

	auto vsBlob = compiled_vs_shaders[(ID3D11VertexShader *)vs.handle];

	HRESULT hResult = d3d11Device->CreateInputLayout(inputElementDesc.data(), inputElementDesc.size(), vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), &d3d11_layout);
	assert(SUCCEEDED(hResult));
	this->input_layouts.push_back(d3d11_layout);
	return d3d11_layout;
}

ID3D11Buffer *D3D11Wrapper::_d3d11_create_vertex_buffer(void *data, size_t n_bytes)
{
	if (data == nullptr)
		return nullptr;
	ID3D11Buffer* vbuffer;

	D3D11_BUFFER_DESC vertexBufferDesc = {};
	vertexBufferDesc.ByteWidth = n_bytes;
	vertexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
	vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

	D3D11_SUBRESOURCE_DATA vertexSubresourceData = {data};

	HRESULT hResult = d3d11Device->CreateBuffer(&vertexBufferDesc, &vertexSubresourceData, &vbuffer);
	assert(SUCCEEDED(hResult));
	this->buffers.push_back(vbuffer);
	return vbuffer;
}

// compile and create vertex + pixel shaders
void D3D11Wrapper::_d3d11_create_shaders(std::wstring vs_path, std::wstring ps_path)
{
	// Create Vertex Shader
	ID3DBlob *vsBlob;
	{
		ID3DBlob *shaderCompileErrorsBlob;
		HRESULT hResult = D3DCompileFromFile(vs_path.c_str(), nullptr, nullptr, "vs_main", "vs_5_0", D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION, 1, &vsBlob, &shaderCompileErrorsBlob);
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
		HRESULT hResult = D3DCompileFromFile(ps_path.c_str(), nullptr, nullptr, "ps_main", "ps_5_0", D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION, 0, &psBlob, &shaderCompileErrorsBlob);
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
				{"NORMAL", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
				{"TEX", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
			};

		HRESULT hResult = d3d11Device->CreateInputLayout(inputElementDesc, ARRAYSIZE(inputElementDesc), vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), &inputLayout);
		assert(SUCCEEDED(hResult));
		vsBlob->Release();
	}

	// Create Vertex and Index Buffer
	{
		std::vector<Vertex_attribute> verts{};
		for (auto &v : state->m_model_original.m_gpu.verts)
		{
			verts.push_back(v);
		}

		D3D11_BUFFER_DESC vertexBufferDesc = {};
		auto vs = sizeof(Vertex_attribute);
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
	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_BORDER;
	samplerDesc.BorderColor[0] = 1.0f;
	samplerDesc.BorderColor[1] = 1.0f;
	samplerDesc.BorderColor[2] = 1.0f;
	samplerDesc.BorderColor[3] = 1.0f;
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;

	d3d11Device->CreateSamplerState(&samplerDesc, &samplerState);
}

std::pair<ID3D11Texture2D*, ID3D11ShaderResourceView *>
D3D11Wrapper::_d3d11_create_texture(Texture& t)
{
	auto [tex, view] =  _d3d11_create_texture(t.width, t.height, TEXTURE_BIND_FLAGS_COLOR_TEXTURE, t.data, t.bytes_per_pixel);
	t.texture_handle = (void*) tex;
	t.view_handle = (void*) view;
	return {tex , view};
}

std::pair<ID3D11Texture2D*, ID3D11ShaderResourceView *>
D3D11Wrapper::_d3d11_create_texture(size_t width, size_t height, TEXTURE_BIND_FLAGS flags, void *data, size_t bytes_per_pixel)
{
	ID3D11Texture2D *texture;
	ID3D11ShaderResourceView *srv;

	// Create Texture
	D3D11_TEXTURE2D_DESC textureDesc = {};
	textureDesc.Width = width;
	textureDesc.Height = height;
	textureDesc.MipLevels = 1;
	textureDesc.ArraySize = 1;
	textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.Usage = D3D11_USAGE_IMMUTABLE;
	textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	if (flags == TEXTURE_BIND_FLAGS_RENDER_TARGET)
	{
		textureDesc.Usage = D3D11_USAGE_DEFAULT;
		textureDesc.BindFlags |= D3D11_BIND_RENDER_TARGET;
	}

	if (data)
	{
		D3D11_SUBRESOURCE_DATA textureSubresourceData = {};
		textureSubresourceData.pSysMem = data;
		textureSubresourceData.SysMemPitch = bytes_per_pixel * width;
		d3d11Device->CreateTexture2D(&textureDesc, &textureSubresourceData, &texture);
	}
	else
	{
		d3d11Device->CreateTexture2D(&textureDesc, nullptr, &texture);
	}
	
	d3d11Device->CreateShaderResourceView(texture, nullptr, &srv);

	this->textures.push_back(texture);
	this->texture_views.push_back(srv);

	return {texture, srv};
}

std::pair<ID3D11Texture2D*, ID3D11DepthStencilView *>
D3D11Wrapper::_d3d11_create_depth_texture(size_t width, size_t height, size_t bytes_per_pixel)
{
	ID3D11Texture2D *texture;
	ID3D11DepthStencilView *dsv;

	// Create Texture
	D3D11_TEXTURE2D_DESC textureDesc = {};
	textureDesc.Width = width;
	textureDesc.Height = height;
	textureDesc.MipLevels = 1;
	textureDesc.ArraySize = 1;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;
	textureDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	textureDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;

	d3d11Device->CreateTexture2D(&textureDesc, nullptr, &texture);
	this->textures.push_back(texture);

	d3d11Device->CreateDepthStencilView(texture, nullptr, &dsv);
	this->texture_views.push_back(dsv);

	return {texture, dsv};
}

std::pair<ID3D11Texture2D *, ID3D11ShaderResourceView *>
D3D11Wrapper::_d3d11_create_texture_cube(size_t width, size_t height, size_t bytes_per_pixel, void *data[6])
{
	ID3D11Texture2D *texture{};
	ID3D11ShaderResourceView *view{};
	// Create Texture
	D3D11_TEXTURE2D_DESC textureDesc = {};
	textureDesc.Width = width;
	textureDesc.Height = height;
	textureDesc.MipLevels = 1;
	textureDesc.ArraySize = 6;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.SampleDesc.Quality = 0;
	textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.Usage = D3D11_USAGE_IMMUTABLE;
	textureDesc.CPUAccessFlags = 0;
	textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	textureDesc.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE;

	D3D11_SHADER_RESOURCE_VIEW_DESC SRViewDesc;
	SRViewDesc.Format = textureDesc.Format;
	SRViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
	SRViewDesc.TextureCube.MipLevels = textureDesc.MipLevels;
	SRViewDesc.TextureCube.MostDetailedMip = 0;

	D3D11_SUBRESOURCE_DATA textureCubeSubresourceData[6];

	textureCubeSubresourceData[0].pSysMem = data[0];
	textureCubeSubresourceData[0].SysMemPitch = bytes_per_pixel * width;
	textureCubeSubresourceData[0].SysMemSlicePitch = 0;

	textureCubeSubresourceData[1].pSysMem = data[1];
	textureCubeSubresourceData[1].SysMemPitch = bytes_per_pixel * width;
	textureCubeSubresourceData[1].SysMemSlicePitch = 0;

	textureCubeSubresourceData[2].pSysMem = data[2];
	textureCubeSubresourceData[2].SysMemPitch = bytes_per_pixel * width;
	textureCubeSubresourceData[2].SysMemSlicePitch = 0;

	textureCubeSubresourceData[3].pSysMem = data[3];
	textureCubeSubresourceData[3].SysMemPitch = bytes_per_pixel * width;
	textureCubeSubresourceData[3].SysMemSlicePitch = 0;

	textureCubeSubresourceData[4].pSysMem = data[4];
	textureCubeSubresourceData[4].SysMemPitch = bytes_per_pixel * width;
	textureCubeSubresourceData[4].SysMemSlicePitch = 0;

	textureCubeSubresourceData[5].pSysMem = data[5];
	textureCubeSubresourceData[5].SysMemPitch = bytes_per_pixel * width;
	textureCubeSubresourceData[5].SysMemSlicePitch = 0;

	d3d11Device->CreateTexture2D(&textureDesc, &textureCubeSubresourceData[0], &texture);
	d3d11Device->CreateShaderResourceView(texture, &SRViewDesc, &view);
	this->textures.push_back(texture);
	this->texture_views.push_back(view);
	return {texture, view};
}

ID3D11Buffer *D3D11Wrapper::_d3d11_create_cbuffer(uint32_t size)
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
	this->buffers.push_back(constantBuffer);
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
	rasterizerDesc.CullMode = D3D11_CULL_BACK;
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
	if (enableDebugLayer)
		_d3d11_set_debug_layer();
	_d3d11_create_swapchain();
	// std::tie(renderTexture, renderTextureView, renderTargetTextureView, std::ignore, renderTargetDepthStencilView) = _d3d11_create_render_texture();
	_d3d11_create_render_target();
	std::tie(envMapVertexShader, envMapPixelShader) = _d3d11_create_env_map_shader(L"../../assets/shaders/envMap.hlsl", "vs_main", "ps_main");
	std::tie(overlayVertexShader, overlayPixelShader) = _d3d11_create_overlay_shader(L"../../assets/shaders/overlay.hlsl", "vs_main", "ps_main");
	_d3d11_create_shaders(L"../../assets/shaders/shaders.hlsl", L"../../assets/shaders/shaders.hlsl");
	_d3d11_create_rasterizer_state();
	_d3d11_create_depth_stencil_state();
	_d3d11_create_sampler_state();
	std::tie(texture, textureView) = _d3d11_create_texture(state->m_model.m_gpu.textures[0]);

	void *cube_texture_face[6] = {
		state->m_model.env_map.right.data,
		state->m_model.env_map.left.data,
		state->m_model.env_map.top.data,
		state->m_model.env_map.bottom.data,
		state->m_model.env_map.front.data,
		state->m_model.env_map.back.data,
	};
	std::tie(envMap, envMapView) = _d3d11_create_texture_cube(
		state->m_model.env_map.front.width,
		state->m_model.env_map.front.height,
		4,
		cube_texture_face);

	cbuffer_0 = _d3d11_create_cbuffer(sizeof(Uniform));
	cbuffer_1 = _d3d11_create_cbuffer(sizeof(PointLight));
	cbuffer_2 = _d3d11_create_cbuffer(sizeof(Material));
	cbuffer_3 = _d3d11_create_cbuffer(sizeof(glm::mat4));
}

void D3D11Wrapper::render_frame(std::vector<Render_Pass> &passes)
{
	FLOAT backgroundColor[4] = {0.1f, 0.2f, 0.6f, 1.0f};
	backgroundColor[0] = backgroundColor[0] >= 1.0f ? 0.0f : backgroundColor[0] + .01f;

	// common to all passes
	{
		d3d11DeviceContext->RSSetState(rasterizerState);
		d3d11DeviceContext->OMSetDepthStencilState(depthStencilState, 0);

		RECT winRect;
		GetClientRect(state->m_window.win32_win, &winRect);
		D3D11_VIEWPORT viewport = {0.0f, 0.0f, (FLOAT)(winRect.right - winRect.left), (FLOAT)(winRect.bottom - winRect.top), 0.0f, 1.0f};
		d3d11DeviceContext->RSSetViewports(1, &viewport);
		d3d11DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		d3d11DeviceContext->ClearRenderTargetView((ID3D11RenderTargetView *)(passes[0].render_target.view_handle), backgroundColor);
		d3d11DeviceContext->ClearDepthStencilView((ID3D11DepthStencilView *)(passes[0].render_target.depth.view_handle), D3D11_CLEAR_DEPTH, 1.0f, 0);
	}
	for (auto &pass : passes)
	{
		auto rtv = (ID3D11RenderTargetView*)pass.render_target.view_handle;
		auto dsv = (ID3D11DepthStencilView*)pass.render_target.depth.view_handle;
		d3d11DeviceContext->OMSetRenderTargets(1, &rtv, dsv);
		d3d11DeviceContext->IASetInputLayout((ID3D11InputLayout *)pass.used_prog.vertex_buffer_layout);

		d3d11DeviceContext->VSSetShader((ID3D11VertexShader *)pass.used_prog.vs.handle, nullptr, 0);
		for (auto &t: pass.used_prog.vs.textures)
		{
			d3d11DeviceContext->VSSetShaderResources(t.binding_point, 1, (ID3D11ShaderResourceView**) &t.view_handle);
			d3d11DeviceContext->VSSetSamplers(t.binding_point, 1, &samplerState);
		}
		for (auto &u: pass.used_prog.vs.uniforms)
		{
			_d3d11_update_cbuffer((ID3D11Buffer*)u.handle, u.data, u.size);
			d3d11DeviceContext->VSSetConstantBuffers(u.binding_point, 1, (ID3D11Buffer **)&u.handle);
		}

		d3d11DeviceContext->PSSetShader((ID3D11PixelShader *)pass.used_prog.ps.handle, nullptr, 0);
		for (auto &t: pass.used_prog.ps.textures)
		{
			d3d11DeviceContext->PSSetShaderResources(t.binding_point, 1, (ID3D11ShaderResourceView**) &t.view_handle);
			d3d11DeviceContext->PSSetSamplers(t.binding_point, 1, &samplerState);
		}
		for (auto &u: pass.used_prog.ps.uniforms)
		{
			_d3d11_update_cbuffer((ID3D11Buffer*)u.handle, u.data, u.size);
			d3d11DeviceContext->PSSetConstantBuffers(u.binding_point, 1, (ID3D11Buffer **)&u.handle);
		}
		d3d11DeviceContext->IASetVertexBuffers(
			0,
			1,
			(ID3D11Buffer **)&pass.used_prog.vertex_buffer,
			(const UINT *)&(pass.used_prog.vertex_buffer_stride),
			(const UINT *)&(pass.used_prog.vertex_buffer_offset));

		d3d11DeviceContext->Draw(pass.used_prog.n_vert_attributes, 0);
	}

	// overlay rendered texture onto swapchain pass
	{
		d3d11DeviceContext->ClearRenderTargetView(d3d11FrameBufferView, backgroundColor);

		d3d11DeviceContext->OMSetRenderTargets(1, &d3d11FrameBufferView, nullptr);

		d3d11DeviceContext->VSSetShader(overlayVertexShader, nullptr, 0);
		d3d11DeviceContext->PSSetShader(overlayPixelShader, nullptr, 0);
		
		// ID3D11ShaderResourceView* srvNULL = {nullptr};
		// d3d11DeviceContext->PSSetShaderResources(0, 1, &srvNULL);
		d3d11DeviceContext->PSSetShaderResources(0, 1, (ID3D11ShaderResourceView **)&passes.back().render_target.color.view_handle);
		d3d11DeviceContext->PSSetSamplers(0, 1, &samplerState);
		d3d11DeviceContext->Draw(6, 0);
		d3d11DeviceContext->OMSetRenderTargets(0, nullptr, nullptr);
	}
	d3d11SwapChain->Present(1, 0);
}

void D3D11Wrapper::cleanup()
{
	for (auto &t : textures)
		t->Release();

	for (auto &v : texture_views)
		v->Release();

	for (auto &b : buffers)
		b->Release();

	for(auto& il: input_layouts)
		il->Release();

	for (auto &[vs, blob] : compiled_vs_shaders)
		blob->Release();

	for (auto &[ps, blob] : compiled_ps_shaders)
		blob->Release();

	for (auto &s : shaders)
		s->Release();

	vertexBuffer->Release();
	// indexBuffer->Release();
	inputLayout->Release();
	vertexShader->Release();
	pixelShader->Release();
	d3d11SwapChain->Release();
	d3d11DeviceContext->Release();
	d3d11Device->Release();
}