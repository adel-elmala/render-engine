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
	// ZoneScoped;
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

	// engine_loop = std::thread(&RenderEngine::render_frame, this);
	// TODO[adel] add to run() in a seperate thread
	// m_win_manager->start_event_loop();
}

void RenderEngine::RenderEngine_init_d3d11(const std::string &model_path)
{
	// ZoneScoped;
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

	m_geometry = std::make_unique<Geometry>();
	m_geometry->bind_state(&state);

	m_d3d11_wrapper = std::make_unique<D3D11Wrapper>();
	m_d3d11_wrapper->bind_state(&state);
	m_d3d11_wrapper->initD3D11();

	// engine_loop = std::thread(&RenderEngine::render_frame, this);
	// TODO[adel] add to run() in a seperate thread
	// m_win_manager->start_event_loop();
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

void RenderEngine::render_frame_software(std::vector<Render_Pass> passes)
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

void RenderEngine::render_frame_d3d11(std::vector<Render_Pass> passes)
{
	// ZoneScoped;
	auto start = std::chrono::system_clock::now();
	state.m_model.m_gpu = state.m_model_original.m_gpu;

	// if (state.m_window.resized) ;
	m_d3d11_wrapper->render_frame(passes);

	auto end = std::chrono::system_clock::now();
	auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
	std::cout << "\rFPS: " << 1000000 / elapsed.count();
	// FrameMark;
}

void RenderEngine::render_frame()
{
	switch (state.backend)
	{
	case BACKEND_SOFTWARE:
		engine_loop = std::thread(&RenderEngine::render_frame_software, this, passes);
		break;
	case BACKEND_D3D11:
		engine_loop = std::thread(&RenderEngine::render_frame_d3d11, this, passes);
		break;
	case BACKEND_VULKAN:
		break;
	default:
		break;
	}
}

void RenderEngine::flush_frame()
{
	engine_loop.join();
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
		// engine_loop.join();
		free(state.m_swapchain.back_buffer);
		free(state.m_swapchain.front_buffer);
		free(state.m_swapchain.z_buffer);
	}
	else if (state.backend == BACKEND_D3D11)
	{
		// engine_loop.join();
		m_d3d11_wrapper->cleanup();
	}
}

void RenderEngine::init_camera()
{
	// ZoneScoped;
	if (state.backend == BACKEND_D3D11)
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
	// ZoneScoped;
	if (state.backend == BACKEND_D3D11)
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
	// ZoneScoped;
	state.m_mode = mode;
}

void RenderEngine::init_swapchain()
{
	// ZoneScoped;
	std::unique_lock lock(state.m_swapchain.m);
	state.m_swapchain.back_buffer = (char *)malloc(state.m_window.height * state.m_window.width * state.m_window.bytes_per_pixel);
	state.m_swapchain.front_buffer = (char *)malloc(state.m_window.height * state.m_window.width * state.m_window.bytes_per_pixel);
	state.m_swapchain.z_buffer = (float *)malloc(state.m_window.height * state.m_window.width * sizeof(float));

	state.m_swapchain.frame_height = state.m_window.height;
	state.m_swapchain.frame_width = state.m_window.width;
	state.m_swapchain.frame_bytes_per_pixel = state.m_window.bytes_per_pixel;

	// reset z_buffer
	auto z_buffer_size = state.m_swapchain.frame_height * state.m_swapchain.frame_width;
	float *z_buffer_end_p = state.m_swapchain.z_buffer + z_buffer_size;
	for (float *start = state.m_swapchain.z_buffer; start < z_buffer_end_p; ++start)
	{
		*start = -1.0f;
	}
}

void RenderEngine::resize_swapchain()
{
	// ZoneScoped;
	std::unique_lock lock(state.m_swapchain.m);
	// free old swap chain
	free(state.m_swapchain.back_buffer);
	free(state.m_swapchain.front_buffer);
	free(state.m_swapchain.z_buffer);
	// allocate new swap chain with new dimenstions
	state.m_swapchain.back_buffer = (char *)malloc(state.m_window.height * state.m_window.width * state.m_window.bytes_per_pixel);
	state.m_swapchain.front_buffer = (char *)malloc(state.m_window.height * state.m_window.width * state.m_window.bytes_per_pixel);
	state.m_swapchain.z_buffer = (float *)malloc(state.m_window.height * state.m_window.width * sizeof(float));

	state.m_swapchain.frame_height = state.m_window.height;
	state.m_swapchain.frame_width = state.m_window.width;
	state.m_swapchain.frame_bytes_per_pixel = state.m_window.bytes_per_pixel;

	state.m_window.resized = false;
}

void RenderEngine::present_swapchain()
{
	// ZoneScoped;
	std::scoped_lock lock(state.m_swapchain.m, state.m_window.m);

	// swap front and back buffers
	std::swap(state.m_swapchain.front_buffer, state.m_swapchain.back_buffer);

	// set back buffer with clear color
	auto swapchain_size = state.m_swapchain.frame_height * state.m_swapchain.frame_width * state.m_swapchain.frame_bytes_per_pixel;
	memset(state.m_swapchain.back_buffer, 0x69, swapchain_size);

	// reset z_buffer
	auto z_buffer_size = state.m_swapchain.frame_height * state.m_swapchain.frame_width;
	float *z_buffer_end_p = state.m_swapchain.z_buffer + z_buffer_size;
	for (float *start = state.m_swapchain.z_buffer; start < z_buffer_end_p; ++start)
	{
		*start = -1.0f;
	}

	auto win_surface_size = state.m_window.height * state.m_window.width * state.m_window.bytes_per_pixel;

	// copy frontbuffer to window surface
	auto smaller_size = std::min(win_surface_size, swapchain_size);
	memcpy(state.m_window.surface, state.m_swapchain.front_buffer, smaller_size);

	m_win_manager->update_surface();
}

Render_Pass RenderEngine::create_render_pass(Program &p, Render_Target &render_target, std::wstring name)
{
	Render_Pass pass{};
	pass.used_prog = p;
	pass.render_target = render_target;
	pass.name = name;
	passes.push_back(pass);

	return pass;
}

Program RenderEngine::create_program(Shader &vs, Shader &ps, Input_Layout &layout, void *vertex_buffer_data, size_t buffer_size, size_t vb_stride, size_t vb_offset, size_t n_vert_attributes)
{
	Program p{};
	p.vs = vs;
	p.ps = ps;
	if (state.backend == BACKEND_D3D11)
	{
		p.vertex_buffer_layout = (void *)m_d3d11_wrapper->_d3d11_create_input_layout(vs, layout);
		p.vertex_buffer = (void *)m_d3d11_wrapper->_d3d11_create_vertex_buffer(vertex_buffer_data, buffer_size);
		p.vertex_buffer_stride = vb_stride;
		p.vertex_buffer_offset = vb_offset;
		p.n_vert_attributes = n_vert_attributes;
	}
	return p;
}

Shader RenderEngine::create_shader(std::wstring path, std::string entry, SHADER_STAGE stage, std::vector<Uniform> &uniforms, std::vector<Texture> &textures)
{
	Shader s{};
	s.path = path;
	s.entry = entry;
	s.uniforms = uniforms;
	s.textures = textures;
	if (state.backend == BACKEND_D3D11)
	{
		if (stage == SHADER_STAGE_VERTEX)
			s.handle = (void *)m_d3d11_wrapper->_d3d11_create_vertex_shader(path, entry);
		else
			s.handle = (void *)m_d3d11_wrapper->_d3d11_create_pixel_shader(path, entry);
	}
	return s;
}

Uniform RenderEngine::create_uniform(const char *name, void *data, size_t size, size_t binding_point)
{
	Uniform u{};
	u.name = name;
	u.size = size;
	u.data = data;
	u.binding_point = binding_point;
	if (state.backend == BACKEND_D3D11)
	{
		u.handle = (void *)m_d3d11_wrapper->_d3d11_create_cbuffer(size);
	}
	return u;
}

Texture RenderEngine::create_texture(const char *name, Texture::DIM dimensions, char *data[6], int width, int height, int bytes_per_pixel, size_t size, size_t binding_point)
{
	Texture t{};
	t.name = name;
	t.binding_point = binding_point;
	t.width = width;
	t.height = height;
	t.bytes_per_pixel = bytes_per_pixel;
	t.data[0] = data[0];
	t.data[1] = data[1];
	t.data[2] = data[2];
	t.data[3] = data[3];
	t.data[4] = data[4];
	t.data[5] = data[5];
	if (state.backend == BACKEND_D3D11)
	{
		if (dimensions == Texture::DIM_CUBE)
		{
			auto [tcube, vcube] = m_d3d11_wrapper->_d3d11_create_texture_cube(width, height, bytes_per_pixel, data);
			t.texture_handle = (void *)tcube;
			t.view_handle = (void *)vcube;
		}
		else if (dimensions == Texture::DIM_2D)
			m_d3d11_wrapper->_d3d11_create_texture(t);
	}
	return t;
}

Render_Target RenderEngine::create_render_target(const char *name, int width, int height, int bytes_per_pixel)
{
	Render_Target rt{};
	rt.name = name;

	Texture color{};
	color.name = "color";
	color.width = width;
	color.height = height;
	color.bytes_per_pixel = bytes_per_pixel;

	Texture depth{};
	depth.name = "color";
	depth.width = width;
	depth.height = height;
	depth.bytes_per_pixel = bytes_per_pixel;
	if (state.backend == BACKEND_D3D11)
	{
		std::tie(color.texture_handle, color.view_handle, rt.view_handle, depth.texture_handle, depth.view_handle) = m_d3d11_wrapper->_d3d11_create_render_texture(width, height, bytes_per_pixel);
	}
	rt.color = color;
	rt.depth = depth;

	return rt;
}