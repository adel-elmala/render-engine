#pragma once

#include <memory>
#include <string>
#include "common.h"

class WindowManager;
class Application;
class Geometry;
class Rasterizer;


// TODO[adel] : use strategy design pattern to switch between rasterizer/ray tracer/vulkan 
class RenderEngine
{
public:
	RenderEngine(const std::string& model_path);
	~RenderEngine();
	void set_drawing_mode(DRAWING_MODE mode);
	void start_engine();

private:
	void init_camera();
	void init_view_volume();
	void init_swapchain();
	void resize_swapchain();
	void present_swapchain();
	
	std::unique_ptr<WindowManager> m_win_manager;
	std::unique_ptr<Application> m_application;
	std::unique_ptr<Geometry> m_geometry;
	std::unique_ptr<Rasterizer> m_rasterizer;
	std::thread engine_loop;
	Engine_State state;
};
