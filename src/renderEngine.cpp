#include "../include/renderEngine.h"
#include "../include/windowManager.h"
#include "../include/Application.h"
#include "../include/geometry.h"
#include "../include/rasterizer.h"
#include "../include/d3d11Wrapper.h"

#include <iostream>
#include <chrono>

using namespace std::chrono_literals;

void RenderEngine::RenderEngine_init_software(const std::string &model_path)
{
	//ZoneScoped;
	state.m_window.height = 600;
	state.m_window.width = 800;
	state.m_window.bytes_per_pixel = 4;
	state.running = true;
	init_camera();
	init_view_volume();
	init_swapchain();
	set_drawing_mode(DRAWING_MODE::DRAWING_MODE_TRIANGLES);

	m_win_manager = std::make_unique<WindowManager>();
	m_win_manager->bind_state(&state);
	m_win_manager->run();

	m_application = std::make_unique<Application>(model_path);
	m_application->bind_state(&state);
	m_application->run();

	m_geometry = std::make_unique<Geometry>();
	m_geometry->bind_state(&state);

	m_rasterizer = std::make_unique<Rasterizer>();
	m_rasterizer->bind_state(&state);

	engine_loop = std::thread(&RenderEngine::render_frame, this);
	// TODO[adel] add to run() in a seperate thread
	m_win_manager->start_event_loop();
}

void RenderEngine::RenderEngine_init_d3d11(const std::string &model_path)
{
	//ZoneScoped;
	state.m_window.height = 600;
	state.m_window.width = 800;
	state.m_window.bytes_per_pixel = 4;
	state.running = true;
	init_camera();
	init_view_volume();
	set_drawing_mode(DRAWING_MODE::DRAWING_MODE_TRIANGLES);

	m_win_manager = std::make_unique<WindowManager>();
	m_win_manager->bind_state(&state);
	m_win_manager->run();

	m_application = std::make_unique<Application>(model_path);
	m_application->bind_state(&state);
	m_application->run();

	m_d3d11_wrapper = std::make_unique<D3D11Wrapper>();
	m_d3d11_wrapper->bind_state(&state);
	m_d3d11_wrapper->initD3D11();

	engine_loop = std::thread(&RenderEngine::render_frame, this);
	// TODO[adel] add to run() in a seperate thread
	m_win_manager->start_event_loop();
}

RenderEngine::RenderEngine(BACKEND backend, const std::string &model_path)
{
	state.backend = backend;
	switch (backend)
	{
	case BACKEND_SOFTWARE:
		RenderEngine_init_software(model_path);
		break;
	case BACKEND_D3D11:
		RenderEngine_init_d3d11(model_path);
		break;
	case BACKEND_VULKAN:
		break;
	default:
		break;
	}
}

void RenderEngine::render_frame_software()
{
	// ZoneScoped;
	auto start = std::chrono::system_clock::now();
	state.m_model.m_cpu = state.m_model_original.m_cpu;
	if (state.m_window.resized)
		resize_swapchain();
	m_geometry->run();
	m_rasterizer->run();
	present_swapchain();

	auto end = std::chrono::system_clock::now();
	auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
	// m_win_manager->update_window_title(std::to_string(1000000.0 / elapsed.count()).c_str());
	std::cout << "\rFPS: " << 1000000 / elapsed.count();
	// FrameMark;
}

void RenderEngine::render_frame_d3d11()
{
	// ZoneScoped;
	auto start = std::chrono::system_clock::now();
	state.m_model.m_gpu = state.m_model_original.m_gpu;
	
	// if (state.m_window.resized) ;
	m_d3d11_wrapper->render_frame();

	auto end = std::chrono::system_clock::now();
	auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
	std::cout << "\rFPS: " << 1000000 / elapsed.count();
	// FrameMark;
}

void RenderEngine::render_frame()
{
	while (state.running)
	{
		switch (state.backend)
		{
		case BACKEND_SOFTWARE:
			render_frame_software();
			break;
		case BACKEND_D3D11:
			render_frame_d3d11();
			break;
		case BACKEND_VULKAN:
			break;
		default:
			break;
		}
	}
}

bool RenderEngine::should_exit()
{
	return !(state.running);
}

RenderEngine::~RenderEngine()
{
	// ZoneScoped;
	if (state.backend == BACKEND_SOFTWARE)
	{
		engine_loop.join();
		free(state.m_swapchain.back_buffer);
		free(state.m_swapchain.front_buffer);
		free(state.m_swapchain.z_buffer);
	}
	else if (state.backend == BACKEND_D3D11)
	{
		engine_loop.join();
		m_d3d11_wrapper->cleanup();
	}
}

void RenderEngine::init_camera()
{
	//ZoneScoped;
	if(state.backend == BACKEND_D3D11)
	{
		state.m_camera.position = glm::vec3{0.0f, 0.0f, 0.0f};
		state.m_camera.lookat = glm::vec3{0.0f, 0.0f, 1.0f};
		state.m_camera.up = glm::vec3{0.0f, 1.0f, 0.0f};
		state.m_camera.sensitivity = 3.5f;
	}
	else
	{
		state.m_camera.position = glm::vec3{0.0f, 0.0f, 0.0f};
		state.m_camera.lookat = glm::vec3{0.0f, 0.0f, -1.0f};
		state.m_camera.up = glm::vec3{0.0f, 1.0f, 0.0f};
		state.m_camera.sensitivity = .3f;
	}
}

void RenderEngine::init_view_volume()
{
	//ZoneScoped;
	if(state.backend == BACKEND_D3D11)
	{
		state.m_view_volume.near_plane = 50.0f;
		state.m_view_volume.far_plane = 1500.0f;
		state.m_view_volume.left_plane = -50.0f;
		state.m_view_volume.right_plane = 50.0f;
		state.m_view_volume.top_plane = 50.0f;
		state.m_view_volume.bottom_plane = -50.0f;
	}
	else
	{
		state.m_view_volume.near_plane = -50.0f;
		state.m_view_volume.far_plane = -150.0f;
		state.m_view_volume.left_plane = -50.0f;
		state.m_view_volume.right_plane = 50.0f;
		state.m_view_volume.top_plane = 50.0f;
		state.m_view_volume.bottom_plane = -50.0f;
	}
}

void RenderEngine::set_drawing_mode(DRAWING_MODE mode)
{
	//ZoneScoped;
	state.m_mode = mode;
}

void RenderEngine::init_swapchain()
{
	//ZoneScoped;
	std::unique_lock lock(state.m_swapchain.m);
	state.m_swapchain.back_buffer = (char*)malloc(state.m_window.height * state.m_window.width * state.m_window.bytes_per_pixel);
	state.m_swapchain.front_buffer = (char*)malloc(state.m_window.height * state.m_window.width * state.m_window.bytes_per_pixel);
	state.m_swapchain.z_buffer = (float*)malloc(state.m_window.height * state.m_window.width * sizeof(float));

	state.m_swapchain.frame_height = state.m_window.height;
	state.m_swapchain.frame_width = state.m_window.width;
	state.m_swapchain.frame_bytes_per_pixel = state.m_window.bytes_per_pixel;

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
	//ZoneScoped;
	std::unique_lock lock(state.m_swapchain.m);
	// free old swap chain
	free(state.m_swapchain.back_buffer);
	free(state.m_swapchain.front_buffer);
	free(state.m_swapchain.z_buffer);
	// allocate new swap chain with new dimenstions
	state.m_swapchain.back_buffer = (char*)malloc(state.m_window.height * state.m_window.width * state.m_window.bytes_per_pixel);
	state.m_swapchain.front_buffer = (char*)malloc(state.m_window.height * state.m_window.width * state.m_window.bytes_per_pixel);
	state.m_swapchain.z_buffer = (float*)malloc(state.m_window.height * state.m_window.width * sizeof(float));

	state.m_swapchain.frame_height = state.m_window.height;
	state.m_swapchain.frame_width = state.m_window.width;
	state.m_swapchain.frame_bytes_per_pixel = state.m_window.bytes_per_pixel;

	state.m_window.resized = false;
}

void RenderEngine::present_swapchain()
{
	//ZoneScoped;
	std::scoped_lock lock(state.m_swapchain.m, state.m_window.m);

	// swap front and back buffers
	std::swap(state.m_swapchain.front_buffer, state.m_swapchain.back_buffer);

	// set back buffer with clear color
	auto swapchain_size = state.m_swapchain.frame_height * state.m_swapchain.frame_width * state.m_swapchain.frame_bytes_per_pixel;
	memset(state.m_swapchain.back_buffer, 0x69, swapchain_size);

	// reset z_buffer
	auto z_buffer_size = state.m_swapchain.frame_height * state.m_swapchain.frame_width;
	float* z_buffer_end_p = state.m_swapchain.z_buffer + z_buffer_size;
	for (float* start = state.m_swapchain.z_buffer; start < z_buffer_end_p; ++start)
	{
		*start = -1.0f;
	}

	auto win_surface_size = state.m_window.height * state.m_window.width * state.m_window.bytes_per_pixel;

	// copy frontbuffer to window surface
	auto smaller_size = std::min(win_surface_size, swapchain_size);
	memcpy(state.m_window.surface, state.m_swapchain.front_buffer, smaller_size);

	m_win_manager->update_surface();
}
