#pragma once

#include <memory>
#include <string>
#include "common.h"

class WindowManager;
class Application;
// TODO[adel] : use strategy design pattern to switch between rasterizer/ray tracer/vulkan 


class RenderEngine
{
public:
	RenderEngine(const std::string& model_path);
	~RenderEngine();
private:
	std::shared_ptr<WindowManager> m_win_manager;
	std::unique_ptr<Application> m_application;

	Engine_State state;
};
