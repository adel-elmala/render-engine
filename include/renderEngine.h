#pragma once

#include <memory>
#include <string>
#include "common.h"

class WindowManager;
class Application;
class Geometry;
class Rasterizer;
class D3D11Wrapper;


// TODO[adel] : use strategy design pattern to switch between rasterizer/ray tracer/vulkan 
class RenderEngine
{
public:
	RenderEngine(BACKEND backend, const std::string &model_path);
	~RenderEngine();
	void set_drawing_mode(DRAWING_MODE mode);
	void render_frame();
	bool should_exit();

private:
	void RenderEngine_init_software(const std::string &model_path);
	void RenderEngine_init_d3d11(const std::string &model_path);
	void render_frame_software();
	void render_frame_d3d11();

	void init_camera();
	void init_view_volume();
	void init_swapchain();
	void resize_swapchain();
	void present_swapchain();
	
	std::unique_ptr<WindowManager> m_win_manager;
	std::unique_ptr<Application> m_application;
	std::unique_ptr<Geometry> m_geometry;
	std::unique_ptr<Rasterizer> m_rasterizer;
	std::unique_ptr<D3D11Wrapper> m_d3d11_wrapper;
	std::thread engine_loop;
	Engine_State state;
};
