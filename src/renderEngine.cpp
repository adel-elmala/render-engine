#include "../include/renderEngine.h"
#include "../include/windowManager.h"
#include "../include/Application.h"
#include "../include/geometry.h"

#include <SDL.h>
#include <iostream>

void renderFrame(int width, int height, int bytes_per_pixel, void* framebuffer);

RenderEngine::RenderEngine(const std::string& model_path)
{
	init_camera();
	init_view_volume();
	set_drawing_mode(DRAWING_MODE::POINTS);

	state.win_height = 600;
	state.win_width = 800;

	m_win_manager = std::make_shared<WindowManager>();
	m_win_manager->bind_state(&state);
	m_win_manager->set_draw_frame_callback(renderFrame);
	m_win_manager->run();

	m_application = std::make_unique<Application>(model_path);
	m_application->bind_state(&state);
	m_application->run();

	m_geometry = std::make_unique<Geometry>();
	m_geometry->bind_state(&state);
	auto res = m_geometry->transform_model_to_window({ 0.0,0.0,state.m_view_volume.near_plane });

	// TODO[adel] add to run() in a seperate thread
	m_win_manager->start_event_loop();
}

RenderEngine::~RenderEngine()
{
}

void RenderEngine::init_camera()
{
	state.m_camera.position = glm::vec3{ 0.0f,0.0f,0.0f };
	state.m_camera.lookat = glm::vec3{ 0.0f,0.0f,-10.0f };
	state.m_camera.up = glm::vec3{ 0.0f,1.0f,0.0f };
}

void RenderEngine::init_view_volume()
{
	state.m_view_volume.near_plane = -50.0f;
	state.m_view_volume.far_plane = -250.0f;
	state.m_view_volume.left_plane = -100.0f;
	state.m_view_volume.right_plane = 100.0f;
	state.m_view_volume.top_plane = 100.0f;
	state.m_view_volume.bottom_plane = -100.0f;
}

void RenderEngine::set_drawing_mode(DRAWING_MODE mode)
{
	state.m_mode = mode;
}


void renderFrame(int width, int height, int bytes_per_pixel, void* framebuffer)
{
	char* end = (char*)framebuffer + (width * height * bytes_per_pixel);

	for (char* start = (char*)framebuffer; start < end; start += bytes_per_pixel)
	{
		switch (bytes_per_pixel)
		{
		case 1:
			*start = 0x00;
			break;
		case 2:
			*(uint16_t*)start = 0x00ff;
			break;
		case 3:
			*(uint16_t*)start = 0x0000;
			start += 2;
			*start = 0xff;
			break;
		case 4:
			*(uint32_t*)start = (uint32_t)start;//0xffaabbcc;
			break;
		}
	}
}

