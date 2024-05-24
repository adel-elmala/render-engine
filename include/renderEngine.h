#pragma once

#include <memory>
#include <string>
#include "common.h"

class WindowManager;
class Application;
class Geometry;


// TODO[adel] : use strategy design pattern to switch between rasterizer/ray tracer/vulkan 
class RenderEngine
{
public:
	RenderEngine(const std::string& model_path);
	~RenderEngine();
	void init_camera();
	void init_view_volume();
	void set_drawing_mode(DRAWING_MODE mode);

private:
	std::shared_ptr<WindowManager> m_win_manager;
	std::unique_ptr<Application> m_application;
	std::unique_ptr<Geometry> m_geometry;

	Engine_State state;
};
