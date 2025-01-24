#pragma once
#include <GLFW/glfw3.h>
#include <d3d11_1.h>


#include <vector>
#include <string>

class D3D11Wrapper
{
public:
	void run();

private:
	void initD3D11();
	void initWindow();
	void mainLoop();
	void cleanup();

	void _d3d11_create_device();
	void _d3d11_set_debug_layer();
	void _d3d11_create_swapchain();
	void _d3d11_create_render_target();
	void _d3d11_create_shaders(std::wstring vs_path, std::wstring ps_path);

	GLFWwindow *window;
	unsigned int win_width = 800;
	unsigned int win_height = 600;
	// d3d11 handles
	ID3D11Device1 *d3d11Device;
	ID3D11DeviceContext1 *d3d11DeviceContext;
	IDXGISwapChain1 *d3d11SwapChain;
	ID3D11RenderTargetView *d3d11FrameBufferView;
	ID3D11VertexShader *vertexShader;
	ID3D11PixelShader *pixelShader;
	ID3D11InputLayout *inputLayout;
	ID3D11Buffer *vertexBuffer;


#ifdef NDEBUG
	const bool enableDebugLayer = false;
#else
	const bool enableDebugLayer = true;
#endif
};
