#pragma once

#include <d3d11_1.h>
#include <vector>
#include <string>
#include <unordered_map>

#include "../include/common.h"

enum TEXTURE_BIND_FLAGS
{
	TEXTURE_BIND_FLAGS_COLOR_TEXTURE,
	TEXTURE_BIND_FLAGS_RENDER_TARGET,
};

class D3D11Wrapper
{
public:
	void initD3D11();
	void render_frame(std::vector<Render_Pass> &passes);
	void cleanup();
	void bind_state(Engine_State *engine_state)
	{
		if (engine_state)
			state = engine_state;
	}

	void _d3d11_create_device();
	void _d3d11_set_debug_layer();
	void _d3d11_create_swapchain();
	void _d3d11_create_render_target();

	ID3D11VertexShader *_d3d11_create_vertex_shader(std::wstring path, std::string entry);
	ID3D11PixelShader *_d3d11_create_pixel_shader(std::wstring path, std::string entry);

	ID3D11InputLayout *_d3d11_create_input_layout(Shader vs, Input_Layout layout);
	ID3D11Buffer *_d3d11_create_vertex_buffer(void *data, size_t n_bytes);

	std::pair<ID3D11Texture2D *, ID3D11ShaderResourceView *> _d3d11_create_texture(Texture &t);
	std::pair<ID3D11Texture2D *, ID3D11ShaderResourceView *> _d3d11_create_texture(size_t width, size_t height, TEXTURE_BIND_FLAGS flags, char *data, size_t bytes_per_pixel = 4);
	std::pair<ID3D11Texture2D *, ID3D11ShaderResourceView *> _d3d11_create_texture_cube(size_t width, size_t height, size_t bytes_per_pixel, char *data[6]);
	std::pair<ID3D11Texture2D *, ID3D11DepthStencilView *> _d3d11_create_depth_texture(size_t width, size_t height, size_t bytes_per_pixel);
	std::tuple<ID3D11Texture2D *, ID3D11ShaderResourceView *, ID3D11RenderTargetView *, ID3D11Texture2D *, ID3D11DepthStencilView *>
	_d3d11_create_render_texture(size_t width, size_t height, size_t bytes_per_pixel = 4);

	ID3D11Buffer *_d3d11_create_cbuffer(uint32_t size);
	void _d3d11_update_cbuffer(ID3D11Buffer *cbuffer, void *data, uint32_t size);

	void _d3d11_create_rasterizer_state();
	void _d3d11_create_depth_stencil_state();
	void _d3d11_create_sampler_state();

private:
	Engine_State *state;

	// d3d11 handles
	ID3D11Device1 *d3d11Device;
	ID3D11DeviceContext1 *d3d11DeviceContext;
	IDXGISwapChain1 *d3d11SwapChain;
	ID3D11RenderTargetView *d3d11FrameBufferView;
	// ID3D11DepthStencilView *d3d11DepthStencilView;
	ID3D11VertexShader *overlayVertexShader;
	ID3D11PixelShader *overlayPixelShader;
	ID3D11SamplerState *samplerState;
	ID3D11RasterizerState *rasterizerState;
	ID3D11DepthStencilState *depthStencilState;

	// resources
	std::vector<ID3D11Resource *> textures;
	std::vector<ID3D11View *> texture_views;
	std::vector<ID3D11Buffer *> buffers;
	std::vector<ID3D11DeviceChild *> shaders;
	std::vector<ID3D11InputLayout *> input_layouts;
	std::unordered_map<ID3D11VertexShader *, ID3DBlob *> compiled_vs_shaders;
	std::unordered_map<ID3D11PixelShader *, ID3DBlob *> compiled_ps_shaders;

#ifdef NDEBUG
	const bool enableDebugLayer = false;
#else
	const bool enableDebugLayer = true;
#endif
};
