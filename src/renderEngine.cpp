#include "../include/renderEngine.h"
#include "../include/windowManager.h"
#include "../include/Application.h"
#include "../include/geometry.h"
#include "../include/rasterizer.h"

#include <iostream>
#include <chrono>
using namespace std::chrono_literals;


RenderEngine::RenderEngine(const std::string& model_path)
{
	state.m_window.win_height = 600;
	state.m_window.win_width = 800;
	state.m_window.win_bytes_per_pixel = 4;
	state.running = true;
	init_camera();
	init_view_volume();
	init_swapchain();
	set_drawing_mode(DRAWING_MODE::TRIANGLES);

	m_win_manager = std::make_unique<WindowManager>();
	m_win_manager->bind_state(&state);
	m_win_manager->run();

	m_application = std::make_unique<Application>(model_path);
	m_application->bind_state(&state);

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
	m_application->run();
	
	while (state.running)
	{
		auto start = std::chrono::system_clock::now();

		// render frame
		state.m_model = state.m_model_original;
		if (state.m_window.win_resized)
			resize_swapchain();
		m_geometry->run();
		m_rasterizer->run();
		present_swapchain();

		auto end = std::chrono::system_clock::now();
		auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
		//m_win_manager->update_window_title(std::to_string(1000000.0 / elapsed.count()).c_str());
		std::cout << 1000000.0 / elapsed.count() << std::endl;
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
	state.m_view_volume.far_plane = -350.0f;
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
	state.m_swapchain.z_buffer = (float*)malloc(state.m_window.win_height * state.m_window.win_width * sizeof(float));

	state.m_swapchain.frame_height = state.m_window.win_height;
	state.m_swapchain.frame_width = state.m_window.win_width;
	state.m_swapchain.frame_bytes_per_pixel = state.m_window.win_bytes_per_pixel;

	// reset z_buffer
	auto z_buffer_size = state.m_swapchain.frame_height * state.m_swapchain.frame_width;
	float* z_buffer_end_p = state.m_swapchain.z_buffer + z_buffer_size;
	for (float* start = state.m_swapchain.z_buffer; start < z_buffer_end_p; ++start)
	{
		*start = -1.0f;
	}
}

void RenderEngine::resize_swapchain()
{
	std::unique_lock lock(state.m_swapchain.m);
	// free old swap chain
	free(state.m_swapchain.back_buffer);
	free(state.m_swapchain.front_buffer);
	free(state.m_swapchain.z_buffer);
	// allocate new swap chain with new dimenstions
	state.m_swapchain.back_buffer = (char*)malloc(state.m_window.win_height * state.m_window.win_width * state.m_window.win_bytes_per_pixel);
	state.m_swapchain.front_buffer = (char*)malloc(state.m_window.win_height * state.m_window.win_width * state.m_window.win_bytes_per_pixel);
	state.m_swapchain.z_buffer = (float*)malloc(state.m_window.win_height * state.m_window.win_width * sizeof(float));

	state.m_swapchain.frame_height = state.m_window.win_height;
	state.m_swapchain.frame_width = state.m_window.win_width;
	state.m_swapchain.frame_bytes_per_pixel = state.m_window.win_bytes_per_pixel;

	state.m_window.win_resized = false;
}

void RenderEngine::present_swapchain()
{
	std::scoped_lock lock(state.m_swapchain.m, state.m_window.m);

	// swap front and back buffers
	std::swap(state.m_swapchain.front_buffer, state.m_swapchain.back_buffer);

	// set back buffer with clear color
	auto swapchain_size = state.m_swapchain.frame_height * state.m_swapchain.frame_width * state.m_swapchain.frame_bytes_per_pixel;
	memset(state.m_swapchain.back_buffer, 0x00, swapchain_size);

	// reset z_buffer
	auto z_buffer_size = state.m_swapchain.frame_height * state.m_swapchain.frame_width;
	float* z_buffer_end_p = state.m_swapchain.z_buffer + z_buffer_size;
	for (float* start = state.m_swapchain.z_buffer; start < z_buffer_end_p; ++start)
	{
		*start = -1.0f;
	}

	auto win_surface_size = state.m_window.win_height * state.m_window.win_width * state.m_window.win_bytes_per_pixel;
	// copy frontbuffer to window surface
	auto smaller_size = std::min(win_surface_size, swapchain_size);
	memcpy(state.m_window.win_surface, state.m_swapchain.front_buffer, smaller_size);

	m_win_manager->update_surface();
}
