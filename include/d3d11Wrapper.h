#pragma once

#include <d3d11_1.h>
#include <vector>
#include <string>

#include "../include/common.h"

class D3D11Wrapper
{
public:
	void initD3D11();
	void render_frame();
	void cleanup();
	void bind_state(Engine_State* engine_state) { if (engine_state) state = engine_state; }

private:
	void _d3d11_create_device();
	void _d3d11_set_debug_layer();
	void _d3d11_create_swapchain();
	void _d3d11_create_render_texture();
	void _d3d11_create_render_target();
	void _d3d11_create_overlay_shader(std::wstring vs_path, std::wstring ps_path);
	void _d3d11_create_env_map_shader(std::wstring vs_path, std::wstring ps_path);
	void _d3d11_create_shaders(std::wstring vs_path, std::wstring ps_path);
	void _d3d11_create_sampler_state();
	void _d3d11_create_texture(Texture t);
	void _d3d11_create_env_map();
	ID3D11Buffer* _d3d11_create_cbuffer(uint32_t size);
	void _d3d11_update_cbuffer(ID3D11Buffer *cbuffer, void *data, uint32_t size);
	void _d3d11_create_rasterizer_state();
	void _d3d11_create_depth_stencil_state();

	Engine_State* state;
	
	// d3d11 handles
	ID3D11Device1 *d3d11Device;
	ID3D11DeviceContext1 *d3d11DeviceContext;
	IDXGISwapChain1 *d3d11SwapChain;
	ID3D11RenderTargetView *renderTargetTextureView;
	ID3D11DepthStencilView *renderTargetDepthStencilView;
	ID3D11RenderTargetView *d3d11FrameBufferView;
	ID3D11DepthStencilView *d3d11DepthStencilView;
	ID3D11VertexShader *envMapVertexShader;
	ID3D11VertexShader *vertexShader;
	ID3D11VertexShader *overlayVertexShader;
	ID3D11PixelShader *envMapPixelShader;
	ID3D11PixelShader *pixelShader;
	ID3D11PixelShader *overlayPixelShader;
	ID3D11InputLayout *inputLayout;
	ID3D11Buffer *vertexBuffer;
	ID3D11Buffer *indexBuffer;
	ID3D11Buffer *cbuffer_0;
	ID3D11Buffer *cbuffer_1;
	ID3D11Buffer *cbuffer_2;
	ID3D11Buffer *cbuffer_3;
	ID3D11Texture2D *texture;
	ID3D11Texture2D *envMap;
	ID3D11Texture2D *renderTexture;
	ID3D11ShaderResourceView *textureView;
	ID3D11ShaderResourceView *envMapView;
	ID3D11ShaderResourceView *renderTextureView;
	ID3D11SamplerState *samplerState;
	ID3D11RasterizerState *rasterizerState;
	ID3D11DepthStencilState *depthStencilState;



#ifdef NDEBUG
	const bool enableDebugLayer = false;
#else
	const bool enableDebugLayer = true;
#endif
};
