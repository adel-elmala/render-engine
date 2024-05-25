#include "../include/renderEngine.h"
#include "../include/windowManager.h"
#include "../include/Application.h"
#include "../include/geometry.h"
#include "../include/rasterizer.h"

#include <SDL.h>
#include <iostream>
#include <chrono>
using namespace std::chrono_literals;

void renderFrame(int width, int height, int bytes_per_pixel, void* framebuffer);

RenderEngine::RenderEngine(const std::string& model_path)
{
	init_camera();
	init_view_volume();
	set_drawing_mode(DRAWING_MODE::POINTS);
	state.m_window.win_height = 600;
	state.m_window.win_width = 800;
	state.m_window.win_bytes_per_pixel = 4;

	m_win_manager = std::make_unique<WindowManager>();
	m_win_manager->bind_state(&state);
	m_win_manager->set_draw_frame_callback(renderFrame);
	m_win_manager->run();

	init_swapchain();

	m_application = std::make_unique<Application>(model_path);
	m_application->bind_state(&state);
	m_application->run();

	m_geometry = std::make_unique<Geometry>();
	m_geometry->bind_state(&state);

	m_rasterizer = std::make_unique<Rasterizer>();
	m_rasterizer->bind_state(&state);

	engine_loop = std::thread(&RenderEngine::start_engine, this);
	// TODO[adel] add to run() in a seperate thread
	m_win_manager->start_event_loop();
}
void RenderEngine::start_engine()
{
	while (state.running)
	{
		state.m_model = state.m_model_original;
		if (state.m_window.win_resized)
			resize_swapchain();
		m_geometry->run();
		m_rasterizer->run();
		present_swapchain();
	}
}

RenderEngine::~RenderEngine()
{
	engine_loop.join();
	free(state.m_swapchain.back_buffer);
	free(state.m_swapchain.front_buffer);
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

void RenderEngine::init_swapchain()
{
	std::unique_lock lock(state.m_swapchain.m);
	state.m_swapchain.back_buffer = (char*)malloc(state.m_window.win_height * state.m_window.win_width * state.m_window.win_bytes_per_pixel);
	state.m_swapchain.front_buffer = (char*)malloc(state.m_window.win_height * state.m_window.win_width * state.m_window.win_bytes_per_pixel);

	state.m_swapchain.frame_height = state.m_window.win_height;
	state.m_swapchain.frame_width = state.m_window.win_width;
	state.m_swapchain.frame_bytes_per_pixel = state.m_window.win_bytes_per_pixel;
}

void RenderEngine::resize_swapchain()
{
	std::unique_lock lock(state.m_swapchain.m);
	// free old swap chain
	free(state.m_swapchain.back_buffer);
	free(state.m_swapchain.front_buffer);
	// allocate new swap chain with new dimenstions
	state.m_swapchain.back_buffer = (char*)malloc(state.m_window.win_height * state.m_window.win_width * state.m_window.win_bytes_per_pixel);
	state.m_swapchain.front_buffer = (char*)malloc(state.m_window.win_height * state.m_window.win_width * state.m_window.win_bytes_per_pixel);

	state.m_swapchain.frame_height = state.m_window.win_height;
	state.m_swapchain.frame_width = state.m_window.win_width;
	state.m_swapchain.frame_bytes_per_pixel = state.m_window.win_bytes_per_pixel;

	state.m_window.win_resized = false;
}

void RenderEngine::present_swapchain()
{
	std::scoped_lock lock(state.m_swapchain.m, state.m_window.m);
	//std::unique_lock lock(state.m_swapchain.m);

	// swap front and back buffers
	std::swap(state.m_swapchain.front_buffer, state.m_swapchain.back_buffer);
	
	// set back buffer with clear color
	auto swapchain_size = state.m_swapchain.frame_height * state.m_swapchain.frame_width * state.m_swapchain.frame_bytes_per_pixel;
	memset(state.m_swapchain.back_buffer, 0x00, swapchain_size);
	
	auto win_surface_size = state.m_window.win_height * state.m_window.win_width * state.m_window.win_bytes_per_pixel;
	// copy frontbuffer to window surface
	auto smaller_size = std::min(win_surface_size, swapchain_size);
	memcpy(state.m_window.win_surface, state.m_swapchain.front_buffer, smaller_size);
	
	m_win_manager->update_surface();
}



void renderFrame(int width, int height, int bytes_per_pixel, void* framebuffer)
{
	//char* end = (char*)framebuffer + (width * height * bytes_per_pixel);

	//for (char* start = (char*)framebuffer; start < end; start += bytes_per_pixel)
	//{
	//	switch (bytes_per_pixel)
	//	{
	//	case 1:
	//		*start = 0x00;
	//		break;
	//	case 2:
	//		*(uint16_t*)start = 0x00ff;
	//		break;
	//	case 3:
	//		*(uint16_t*)start = 0x0000;
	//		start += 2;
	//		*start = 0xff;
	//		break;
	//	case 4:
	//		*(uint32_t*)start = (uint32_t)start;//0xffaabbcc;
	//		break;
	//	}
	//}
}

