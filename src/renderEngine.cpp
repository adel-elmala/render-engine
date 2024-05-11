#include "../include/renderEngine.h"
#include "../include/windowManager.h"
#include "../include/Application.h"

#include <SDL.h>
#include <iostream>

void renderFrame(int width, int height, int bytes_per_pixel, void* framebuffer);

RenderEngine::RenderEngine(const std::string& model_path)
{
	m_win_manager = std::make_shared<WindowManager>();
	m_win_manager->bind_state(&state);
	m_win_manager->set_draw_frame_callback(renderFrame);
	m_win_manager->run();

	m_application = std::make_unique<Application>(model_path);
	m_application->bind_state(&state);
	m_application->run();

	// TODO[adel] add to run() in a seperate thread
	m_win_manager->start_event_loop();
}

RenderEngine::~RenderEngine()
{
}


void renderFrame(int width, int height, int bytes_per_pixel, void* framebuffer)
{
	static char counter = 0;
	memset(framebuffer, 0x00 + (counter++), bytes_per_pixel * width * height);
}

