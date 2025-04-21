#pragma once

#include <memory>
#include <string>
#include "common.h"
#include "geometry.h"
#include "SceneManager.h"
#include "windowManager.h"

class D3D11Wrapper;


struct Engine_Options
{
	bool render_bounding_boxes;
	bool render_lights;
};

// TODO[adel] : use strategy design pattern to switch between rasterizer/ray tracer/vulkan
class RenderEngine
{
public:
	RenderEngine(BACKEND backend);
	~RenderEngine();
	void set_drawing_mode(Render_Pass* pass, DRAWING_MODE mode);
	void scene_add_model(Model m);
	void scene_add_point_light(PointLight l);
	void scene_add_dir_light(DirLight l);
	void scene_update_camera(Camera cam);
	void scene_finish();
	void update_resources();
	void render_frame();
	void render_bounding_boxes(bool on);
	void render_lights(bool on);
	void flush_frame();
	bool should_exit();
	Render_Pass* create_render_pass(Program &p, Render_Target &render_target, std::wstring name);
	Program create_program(Shader &vs, Shader &ps, Input_Layout &layout, void *vertex_buffer_data, size_t buffer_size, size_t vb_stride, size_t vb_offset, size_t n_vert_attributes);
	Shader create_shader(std::wstring path, std::string entry, SHADER_STAGE stage, std::vector<Uniform> &uniforms, std::vector<Texture> &textures, std::function<void()> update);
	Uniform create_uniform(const char *name, void *data, size_t size, size_t binding_point);
	Texture create_texture(const char *name, Texture::DIM dimensions, char *data[6], int width, int height, int bytes_per_pixel, size_t size, size_t binding_point);
	Render_Target create_render_target(const char *name, int width, int height, int bytes_per_pixel);
	Model create_axis_aligned_plane(glm::vec3 normal, glm::vec3 center, size_t width, size_t height);
	
	std::unique_ptr<SceneManager> m_scene_manager;
	std::unique_ptr<WindowManager> m_win_manager;
	
	private:
	void RenderEngine_init_d3d11();
	void render_frame_d3d11(std::vector<Render_Pass*> passes);
	
	void init_camera();
	void init_view_volume();
	void _render_opaques();
	void _render_bounding_boxes();
	void _render_lights();
	std::vector<glm::vec4>* _bounding_box_lines(Bounding_Box bb);
	
	std::unique_ptr<Geometry> m_geometry;
	std::unique_ptr<D3D11Wrapper> m_d3d11_wrapper;
	Engine_State state;
	std::vector<Render_Pass*> passes;
	Render_Target main_rt;
	std::thread engine_loop;
	Engine_Options options;
	std::vector<std::vector<glm::vec4>*> bounding_boxes;
};
