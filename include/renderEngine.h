#pragma once

#include <string>
#include "common.h"
#include "geometry.h"
#include "SceneManager.h"
#include "windowManager.h"

class D3D11Wrapper;

class RenderEngine
{
public:
	RenderEngine(BACKEND backend);
	~RenderEngine();
	void scene_add_model(Model m);
	void scene_add_point_light(PointLight l);
	void scene_add_dir_light(DirLight l);
	void scene_add_skybox(Env_map skybox);
	void scene_update_camera(Camera cam);
	void scene_finish();
	void set_drawing_mode(Render_Pass *pass, DRAWING_MODE mode);
	void set_clear_color(glm::vec4 color);
	void render_bounding_boxes(bool on);
	void render_lights(bool on);
	void render_ground(bool on);
	void render_skybox(bool on);
	void render_shadows(bool on);
	void mirror_ground(bool on);
	void render_frame();
	void flush_frame();
	bool should_exit();

	std::unique_ptr<SceneManager> m_scene_manager;
	std::unique_ptr<WindowManager> m_win_manager;

private:
	void _init_d3d11();
	void _render_frame_d3d11(std::vector<Render_Pass *> passes);

	void _init_camera();
	void _init_view_volume();
	void _render_opaques();
	void _render_opaques_reflected();
	void _render_bounding_boxes();
	void _render_lights();
	void _render_ground();
	void _render_skybox();
	void _render_shadows();
	void _update_resources();
	void _frame_gui();
	void _gen_scene_bounding_box();
	void _resize_render_targets();
	std::vector<glm::vec4> *_bounding_box_lines(Bounding_Box bb);
	Render_Pass *_create_render_pass(Program &p, Render_Target *render_target, std::wstring name, glm::vec4 clear_color);
	Program _create_program(Shader &vs, Shader &ps, Input_Layout &layout, void *vertex_buffer_data, size_t buffer_size, size_t vb_stride, size_t vb_offset, size_t n_vert_attributes);
	Shader _create_shader(std::wstring path, std::string entry, SHADER_STAGE stage, std::vector<Uniform> &uniforms, std::vector<Texture *> textures, std::function<void()> update);
	Uniform _create_uniform(const char *name, void *data, size_t size, size_t binding_point);
	Texture _create_texture(const char *name, Texture::DIM dimensions, char *data[6], int width, int height, int bytes_per_pixel, size_t size, size_t binding_point);
	Render_Target *_create_render_target(const char *name, int width, int height, int bytes_per_pixel);
	Model _create_axis_aligned_plane(glm::vec3 normal, glm::vec3 center, size_t width, size_t height);

	std::unique_ptr<Geometry> m_geometry;
	std::unique_ptr<D3D11Wrapper> m_d3d11_wrapper;
	std::vector<Render_Pass *> passes;
	std::vector<Render_Target *> unique_render_targets;
	std::vector<std::vector<glm::vec4> *> bounding_boxes;
	Render_Target *main_rt;
	Render_Target *depth_rt;
	Render_Target *mirrored_scene_rt;
	Engine_State state;
	Engine_Options options;
	std::thread engine_loop;
	glm::vec4 clear_color;
};
