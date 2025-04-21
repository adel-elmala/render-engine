#include "../include/renderEngine.h"
#include "../include/windowManager.h"
#include "../include/SceneManager.h"
#include "../include/geometry.h"
#include "../include/d3d11Wrapper.h"

#include <iostream>
#include <chrono>

using namespace std::chrono_literals;

void RenderEngine::RenderEngine_init_d3d11()
{
	// ZoneScoped;
	state.window.height = 600;
	state.window.width = 800;
	state.window.bytes_per_pixel = 4;
	state.running = true;
	init_camera();
	init_view_volume();
	set_drawing_mode(DRAWING_MODE::DRAWING_MODE_TRIANGLES);

	m_win_manager = std::make_unique<WindowManager>();
	m_win_manager->bind_state(&state);
	m_win_manager->run();

	m_scene_manager= std::make_unique<SceneManager>();

	m_geometry = std::make_unique<Geometry>();
	m_geometry->bind_state(&state);

	m_d3d11_wrapper = std::make_unique<D3D11Wrapper>();
	m_d3d11_wrapper->bind_state(&state);
	m_d3d11_wrapper->initD3D11();
}

RenderEngine::RenderEngine(BACKEND backend)
{
	state.backend = backend;
	switch (backend)
	{
	case BACKEND_D3D11:
		RenderEngine_init_d3d11();
		break;
	case BACKEND_VULKAN:
		break;
	default:
		break;
	}
}

void RenderEngine::render_frame_d3d11(std::vector<Render_Pass> passes)
{
	// ZoneScoped;
	auto start = std::chrono::system_clock::now();

	// if (state.window.resized) ;
	m_d3d11_wrapper->render_frame(passes);

	auto end = std::chrono::system_clock::now();
	auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
	std::cout << "\rFPS: " << 1000000 / elapsed.count();
	// FrameMark;
}

void RenderEngine::update_resources()
{
	m_geometry->update_world_transform();
	m_geometry->update_camera_transform();
	m_geometry->update_perspective_transform();
	for(auto& pass: passes)
	{
		pass.used_prog.vs.update_uniforms();
		pass.used_prog.ps.update_uniforms();
	}
}

void RenderEngine::render_frame()
{
	update_resources();
	switch (state.backend)
	{
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
	if (state.backend == BACKEND_D3D11)
	{
		// engine_loop.join();
		m_d3d11_wrapper->cleanup();
	}
}

void RenderEngine::scene_add_point_light(PointLight l)
{
	state.scene.pLights.push_back(l);
}

void RenderEngine::scene_add_dir_light(DirLight l)
{
	state.scene.dLights.push_back(l);
}

void RenderEngine::scene_add_model(Model m)
{
	state.scene.models.push_back(m);
}

void RenderEngine::scene_update_camera(Camera cam)
{
	state.scene.cam = cam;
}

void RenderEngine::init_camera()
{
	// ZoneScoped;
	if (state.backend == BACKEND_D3D11)
	{
		state.scene.cam.position = glm::vec3{0.0f, 0.0f, 0.0f};
		state.scene.cam.lookat = glm::vec3{0.0f, 0.0f, 1.0f};
		state.scene.cam.up = glm::vec3{0.0f, 1.0f, 0.0f};
		state.scene.cam.sensitivity = 3.5f;
	}
}

void RenderEngine::init_view_volume()
{
	// ZoneScoped;
	if (state.backend == BACKEND_D3D11)
	{
		state.view_volume.near_plane = 50.0f;
		state.view_volume.far_plane = 1500.0f;
		state.view_volume.left_plane = -50.0f;
		state.view_volume.right_plane = 50.0f;
		state.view_volume.top_plane = 50.0f;
		state.view_volume.bottom_plane = -50.0f;
	}
}

void RenderEngine::set_drawing_mode(DRAWING_MODE mode)
{
	// ZoneScoped;
	state.mode = mode;
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

Shader RenderEngine::create_shader(std::wstring path, std::string entry, SHADER_STAGE stage, std::vector<Uniform> &uniforms, std::vector<Texture> &textures, std::function<void()> update)
{
	Shader s{};
	s.path = path;
	s.entry = entry;
	s.uniforms = uniforms;
	s.textures = textures;
	s.update_uniforms = update;
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
	color.name = "color_target";
	color.width = width;
	color.height = height;
	color.bytes_per_pixel = bytes_per_pixel;

	Texture depth{};
	depth.name = "depth_target";
	depth.width = width;
	depth.height = height;
	depth.bytes_per_pixel = bytes_per_pixel;
	if (state.backend == BACKEND_D3D11)
	{
		std::tie(color.texture_handle, color.view_handle, rt.color_view_handle, depth.texture_handle, depth.view_handle, rt.depth_view_handle) = m_d3d11_wrapper->_d3d11_create_render_texture(width, height, bytes_per_pixel);
	}
	rt.color = color;
	rt.depth = depth;

	return rt;
}


Model RenderEngine::create_axis_aligned_plane(glm::vec3 normal, glm::vec3 center , size_t width, size_t height)
{
	auto n = glm::normalize(normal);
	auto n_dot_xy = glm::dot(glm::vec3(0, 0, 1), n);
	auto n_dot_xz = glm::dot(glm::vec3(0, 1, 0), n);
	auto n_dot_yz = glm::dot(glm::vec3(1, 0, 0), n);

	glm::vec3 lower_left, lower_right, top_left, top_right;
	if (n_dot_xy == 1 || n_dot_xy == -1)
	{
		lower_left = glm::vec3(center.x - width / 2, center.y - height / 2, center.z);
		lower_right = glm::vec3(center.x + width / 2, center.y - height / 2, center.z);
		top_left = glm::vec3(center.x - width / 2, center.y + height / 2, center.z);
		top_right = glm::vec3(center.x + width / 2, center.y + height / 2, center.z);
	}
	else if (n_dot_xz == 1 || n_dot_xz == -1)
	{
		lower_left = glm::vec3(center.x - width / 2, center.y, center.z - height / 2);
		lower_right = glm::vec3(center.x + width / 2, center.y, center.z - height / 2);
		top_left = glm::vec3(center.x - width / 2, center.y, center.z + height / 2);
		top_right = glm::vec3(center.x + width / 2, center.y, center.z + height / 2);
	}
	else if (n_dot_yz == 1 || n_dot_yz == -1)
	{
		lower_left = glm::vec3(center.x, center.y - height / 2, center.z - width / 2);
		lower_right = glm::vec3(center.x, center.y - height / 2, center.z + width / 2);
		top_left = glm::vec3(center.x, center.y + height / 2, center.z - width / 2);
		top_right = glm::vec3(center.x, center.y + height / 2, center.z + width / 2);
	}

	Model m{};

	Vertex_attribute v0{};
	v0.pos = glm::vec4(lower_left, 1.0);
	v0.normal = glm::vec4(n, 0);
	v0.uv = glm::vec2(0, 0);

	Vertex_attribute v1{};
	v1.pos = glm::vec4(lower_right, 1.0);
	v1.normal = glm::vec4(n, 0);
	v1.uv = glm::vec2(1, 0);

	Vertex_attribute v2{};
	v2.pos = glm::vec4(top_right, 1.0);
	v2.normal = glm::vec4(n, 0);
	v2.uv = glm::vec2(1, 1);

	Vertex_attribute v3{};
	v3.pos = glm::vec4(lower_left, 1.0);
	v3.normal = glm::vec4(n, 0);
	v3.uv = glm::vec2(0, 0);

	Vertex_attribute v4{};
	v4.pos = glm::vec4(top_right, 1.0);
	v4.normal = glm::vec4(n, 0);
	v4.uv = glm::vec2(1, 1);

	Vertex_attribute v5{};
	v5.pos = glm::vec4(top_left, 1.0);
	v5.normal = glm::vec4(n, 0);
	v5.uv = glm::vec2(0, 1);

	m.verts.push_back(v0);
	m.verts.push_back(v1);
	m.verts.push_back(v2);
	m.verts.push_back(v3);
	m.verts.push_back(v4);
	m.verts.push_back(v5);
	return m;
}

struct opaque_pass_mats_unifrom
{
	glm::mat4 model_world;
	glm::mat4 world_camera;
	glm::mat4 camera_ndc;
};

void RenderEngine::render_opaques()
{
	auto &plight = state.scene.pLights[0]; // TODO(adel): account for multiple light sources in the scene
	main_rt = create_render_target(
		"",
		state.window.width,
		state.window.height,
		state.window.bytes_per_pixel);

	size_t model_id = 0;
	for (auto& model: state.scene.models)
	{
		auto mats = new opaque_pass_mats_unifrom; // TODO(adel): fix leak
		mats->model_world = m_geometry->model_world_transform;
		mats->world_camera = m_geometry->world_camera_transform;
		mats->camera_ndc = m_geometry->camera_ndc_transform;

		auto vs_uniform_mat = create_uniform("mats", mats, sizeof(opaque_pass_mats_unifrom), 0);
		auto vs_uniform_light = create_uniform("light", &plight, sizeof(PointLight), 1);
		std::vector<Uniform> vs_uniforms = {vs_uniform_mat, vs_uniform_light};
		std::vector<Texture> vs_textures = {};

		auto vs_update = [this, model_id, &model, &plight, mats]()
		{
			opaque_pass_mats_unifrom new_mats{};
			new_mats.model_world = m_geometry->model_world_transform * model.model_world_transfrom;
			new_mats.world_camera = m_geometry->world_camera_transform;
			new_mats.camera_ndc = m_geometry->camera_ndc_transform;
			memcpy(mats, &new_mats, sizeof(opaque_pass_mats_unifrom));
		};

		auto vs = create_shader(
			L"../../assets/shaders/shaders.hlsl",
			"vs_main",
			SHADER_STAGE_VERTEX,
			vs_uniforms,
			vs_textures,
			vs_update);

		std::vector<Uniform> ps_uniforms = {};
		std::vector<Texture> ps_textures = {};
		auto ps_update = [](){};

		auto ps = create_shader(
			L"../../assets/shaders/shaders.hlsl",
			"ps_main",
			SHADER_STAGE_PIXEL,
			ps_uniforms,
			ps_textures,
			ps_update);

		auto _prog = create_program(
			vs,
			ps,
			model.layout,
			model.verts.data(),
			model.verts.size() * sizeof(Vertex_attribute),
			sizeof(Vertex_attribute),
			0,
			model.verts.size());

		create_render_pass(_prog, main_rt, L"pass - render opaques");
	}
}

void RenderEngine::scene_finish()
{
	// opqaue pass
	render_opaques();
}