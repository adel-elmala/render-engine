#pragma once

#include <memory>
#include <string>
#include "common.h"
#include "geometry.h"
#include "Application.h"
#include "windowManager.h"

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
	void flush_frame();
	bool should_exit();
	Render_Pass create_render_pass(Program &p, Render_Target &render_target, std::wstring name);
	Program create_program(Shader &vs, Shader &ps, Input_Layout &layout, void *vertex_buffer_data, size_t buffer_size, size_t vb_stride, size_t vb_offset, size_t n_vert_attributes);
	Shader create_shader(std::wstring path, std::string entry, SHADER_STAGE stage, std::vector<Uniform> &uniforms, std::vector<Texture> &textures);
	Uniform create_uniform(const char *name, void *data, size_t size, size_t binding_point);
	Texture create_texture(const char *name, Texture::DIM dimensions, char *data[6], int width, int height, int bytes_per_pixel, size_t size, size_t binding_point);
	Render_Target create_render_target(const char *name, int width, int height, int bytes_per_pixel);

	std::unique_ptr<Application> m_application;
	std::unique_ptr<WindowManager> m_win_manager;
	std::unique_ptr<Geometry> m_geometry;
	Engine_State state;
	std::vector<Render_Pass> passes;

private:
	void RenderEngine_init_software(const std::string &model_path);
	void RenderEngine_init_d3d11(const std::string &model_path);
	void render_frame_software(std::vector<Render_Pass> passes);
	void render_frame_d3d11(std::vector<Render_Pass> passes);

	void init_camera();
	void init_view_volume();
	void init_swapchain();
	void resize_swapchain();
	void present_swapchain();

	std::unique_ptr<Rasterizer> m_rasterizer;
	std::unique_ptr<D3D11Wrapper> m_d3d11_wrapper;
	std::thread engine_loop;
};
