#pragma once
#include <GLFW/glfw3.h>
#include <d3d11_1.h>


#include <vector>

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

	GLFWwindow *window;
	unsigned int win_width = 800;
	unsigned int win_height = 600;
	// d3d11 handles
	ID3D11Device1 *d3d11Device;
	ID3D11DeviceContext1 *d3d11DeviceContext;
	IDXGISwapChain1 *d3d11SwapChain;
	ID3D11RenderTargetView *d3d11FrameBufferView;

#ifdef NDEBUG
	const bool enableDebugLayer = false;
#else
	const bool enableDebugLayer = true;
#endif
};
