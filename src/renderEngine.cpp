#include "../include/renderEngine.h"
#include "../include/windowManager.h"
#include "../include/SceneManager.h"
#include "../include/geometry.h"
#include "../include/d3d11Wrapper.h"

#include <iostream>
#include <chrono>

using namespace std::chrono_literals;

RenderEngine::RenderEngine(BACKEND backend)
{
	state.backend = backend;
	options = {};
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

RenderEngine::~RenderEngine()
{
	// ZoneScoped;
	if (state.backend == BACKEND_D3D11)
	{
		// engine_loop.join();
		m_d3d11_wrapper->cleanup();
	}
	for (auto pass : passes)
	{
		delete pass;
	}
	for (auto bb: bounding_boxes)
	{
		delete bb;
	}
}

void RenderEngine::RenderEngine_init_d3d11()
{
	// ZoneScoped;
	state.window.height = 600;
	state.window.width = 800;
	state.window.bytes_per_pixel = 4;
	state.running = true;
	init_camera();
	init_view_volume();

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

void RenderEngine::render_frame_d3d11(std::vector<Render_Pass*> passes)
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
		pass->used_prog.vs.update_uniforms();
		pass->used_prog.ps.update_uniforms();
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

void RenderEngine::scene_add_skybox(Env_map skybox)
{
	options.render_skybox = true;
	state.scene.skybox = skybox;
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

void RenderEngine::set_drawing_mode(Render_Pass* pass, DRAWING_MODE mode)
{
	// ZoneScoped;
	pass->mode = mode;
}

Render_Pass* RenderEngine::create_render_pass(Program &p, Render_Target &render_target, std::wstring name, glm::vec4 clear_color)
{
	auto pass = new Render_Pass{};
	pass->used_prog = p;
	pass->render_target = render_target;
	pass->name = name;
	pass->clear_color = clear_color;
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

std::vector<glm::vec4>* RenderEngine::_bounding_box_lines(Bounding_Box bb)
{
	glm::vec4 p0(bb.min_x, bb.min_y, bb.min_z, 1.0f);
	glm::vec4 p1(bb.max_x, bb.min_y, bb.min_z, 1.0f);
	glm::vec4 p2(bb.max_x, bb.max_y, bb.min_z, 1.0f);
	glm::vec4 p3(bb.min_x, bb.max_y, bb.min_z, 1.0f);

	glm::vec4 p4(bb.min_x, bb.min_y, bb.max_z, 1.0f);
	glm::vec4 p5(bb.max_x, bb.min_y, bb.max_z, 1.0f);
	glm::vec4 p6(bb.max_x, bb.max_y, bb.max_z, 1.0f);
	glm::vec4 p7(bb.min_x, bb.max_y, bb.max_z, 1.0f);

	// NOTE(adel): each triangle has 4 points, as the end point is to enclose the tringle when rendering using line strips primitives
	// NOTE(adel): many points can be removed, otherwise lines will be redrawn, but kept this way for simplicity
	auto bb_verts = new std::vector<glm::vec4>{
		p0, p1, p2, p0, // front face - t1
		p0, p2, p3, p0, // front face - t2
		p1, p5, p6, p1, // right face - t1
		p1, p6, p2, p1, // right face - t2
		p4, p5, p6, p4, // back face - t1
		p4, p6, p7, p4, // back face - t2
		p0, p4, p7, p0, // left face - t1
		p0, p7, p3, p0, // left face - t2
		p3, p2, p6, p3, // top face - t1
		p3, p6, p7, p3, // top face - t2
		p0, p1, p5, p0, // bottom face - t1
		p0, p5, p4, p0	// bottom face - t2
	};

	bounding_boxes.push_back(bb_verts);
	return bb_verts;
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

void RenderEngine::_render_bounding_boxes()
{
	for (auto& model: state.scene.models)
	{
		auto mats = new opaque_pass_mats_unifrom; // TODO(adel): fix leak
		mats->model_world = m_geometry->model_world_transform;
		mats->world_camera = m_geometry->world_camera_transform;
		mats->camera_ndc = m_geometry->camera_ndc_transform;

		auto vs_uniform_mat = create_uniform("mats", mats, sizeof(opaque_pass_mats_unifrom), 0);
		std::vector<Uniform> vs_uniforms = {vs_uniform_mat};
		std::vector<Texture> vs_textures = {};

		auto vs_update = [this, &model, mats]()
		{
			opaque_pass_mats_unifrom new_mats{};
			new_mats.model_world = m_geometry->model_world_transform * model.model_world_transfrom;
			new_mats.world_camera = m_geometry->world_camera_transform;
			new_mats.camera_ndc = m_geometry->camera_ndc_transform;
			memcpy(mats, &new_mats, sizeof(opaque_pass_mats_unifrom));
		};

		auto vs = create_shader(
			L"../../assets/shaders/wireframe.hlsl",
			"vs_main",
			SHADER_STAGE_VERTEX,
			vs_uniforms,
			vs_textures,
			vs_update);

		std::vector<Uniform> ps_uniforms = {};
		std::vector<Texture> ps_textures = {};
		auto ps_update = [](){};

		auto ps = create_shader(
			L"../../assets/shaders/wireframe.hlsl",
			"ps_main",
			SHADER_STAGE_PIXEL,
			ps_uniforms,
			ps_textures,
			ps_update);

		Input_Layout layout{};
		Element_Desc e0 = {V_ATTRIBUTE_TYPE_POSITION, FORMAT_R32G32B32A32_FLOAT, V_ATTRIBUTE_FREQ_PER_VERTEX};
		layout.elements = {e0};

		auto bb_verts = _bounding_box_lines(model.bb);

		auto _prog = create_program(
			vs,
			ps,
			layout,
			bb_verts->data(),
			bb_verts->size() * sizeof(glm::vec4),
			sizeof(glm::vec4),
			0,
			bb_verts->size());

		auto pass = create_render_pass(_prog, main_rt, L"pass - render bounding boxes", clear_color);
		set_drawing_mode(pass, DRAWING_MODE_LINES);
	}
}

void RenderEngine::_render_ground()
{
	auto scene_width = state.scene.bb.max_x - state.scene.bb.min_x;
	auto scene_height = state.scene.bb.max_y - state.scene.bb.min_y;
	auto scene_depth = state.scene.bb.max_z - state.scene.bb.min_z;

	auto scene_center = glm::vec3(
		state.scene.bb.min_x + scene_width / 2.0,
		state.scene.bb.min_y,
		state.scene.bb.min_z + scene_depth / 2.0);

	auto plane = create_axis_aligned_plane(glm::vec3(0, 1, 0), scene_center, scene_width * 4, scene_depth * 4);

	auto mats = new opaque_pass_mats_unifrom;
	mats->model_world = m_geometry->model_world_transform;
	mats->world_camera = m_geometry->world_camera_transform;
	mats->camera_ndc = m_geometry->camera_ndc_transform;
	auto vs_uniform_mat = create_uniform("mats", mats, sizeof(opaque_pass_mats_unifrom), 0);

	auto used_plight = state.scene.pLights[0];
	auto scene_center_ws = m_geometry->model_world_transform * glm::vec4(scene_center, 1);
	auto light_dir = glm::vec3(scene_center_ws) - used_plight.position;
	float fovy = atan2f(state.view_volume.top_plane, state.view_volume.near_plane) * 2;

	auto light_mats = new opaque_pass_mats_unifrom;
	light_mats->model_world = m_geometry->model_world_transform;
	light_mats->world_camera = glm::lookAtLH(used_plight.position, light_dir, glm::vec3(0, 1, 0));
	light_mats->camera_ndc = glm::perspectiveLH(fovy, (float)state.window.width / state.window.height, state.view_volume.near_plane, state.view_volume.far_plane);
	auto vs_uniform_mat_2 = create_uniform("light_mats", light_mats, sizeof(opaque_pass_mats_unifrom), 1);

	std::vector<Uniform> vs_uniforms = {vs_uniform_mat, vs_uniform_mat_2};
	std::vector<Texture> vs_textures = {};

	auto vs_update = [this, mats, light_mats, scene_center, fovy, used_plight]()
	{
		opaque_pass_mats_unifrom new_mats{};
		new_mats.model_world = m_geometry->model_world_transform;
		new_mats.world_camera = m_geometry->world_camera_transform;
		new_mats.camera_ndc = m_geometry->camera_ndc_transform;
		memcpy(mats, &new_mats, sizeof(opaque_pass_mats_unifrom));

		auto scene_center_ws = m_geometry->model_world_transform * glm::vec4(scene_center, 1);
		auto light_dir = glm::vec3(scene_center_ws) - used_plight.position;

		opaque_pass_mats_unifrom new_mats_light{};
		new_mats_light.model_world = m_geometry->model_world_transform;
		new_mats_light.world_camera = glm::lookAtLH(used_plight.position, light_dir, glm::vec3(0, 1, 0));
		new_mats_light.camera_ndc = glm::perspectiveLH(fovy, (float)state.window.width / state.window.height, state.view_volume.near_plane, state.view_volume.far_plane);

		memcpy(light_mats, &new_mats_light, sizeof(opaque_pass_mats_unifrom));
	};

	auto vs = create_shader(
		L"../../assets/shaders/mirror_reflection.hlsl",
		"vs_main",
		SHADER_STAGE_VERTEX,
		vs_uniforms,
		vs_textures,
		vs_update);

	auto ps_uniform_mirror_option = create_uniform("mirror", &options.ground_is_mirror, sizeof(bool), 0);
	auto ps_uniform_shadow_option = create_uniform("shadow", &options.render_shadows, sizeof(bool), 1);

	std::vector<Uniform> ps_uniforms = {ps_uniform_mirror_option, ps_uniform_shadow_option};
	std::vector<Texture> ps_textures = {};
	if (options.ground_is_mirror)
	{
		auto reflected_scene = mirrored_scene_rt.color;
		reflected_scene.binding_point = 0;
		ps_textures.push_back(reflected_scene);
	}
	if (options.render_shadows)
	{
		auto depth_map = depth_rt.depth;
		depth_map.binding_point = 1;
		ps_textures.push_back(depth_map);
	}

	auto ps_update = [](){};

	auto ps = create_shader(
		L"../../assets/shaders/mirror_reflection.hlsl",
		"ps_main",
		SHADER_STAGE_PIXEL,
		ps_uniforms,
		ps_textures,
		ps_update);

	Input_Layout layout{};
	Element_Desc e0 = {V_ATTRIBUTE_TYPE_POSITION, FORMAT_R32G32B32A32_FLOAT, V_ATTRIBUTE_FREQ_PER_VERTEX};
	Element_Desc e1 = {V_ATTRIBUTE_TYPE_NORMAL, FORMAT_R32G32B32A32_FLOAT, V_ATTRIBUTE_FREQ_PER_VERTEX};
	Element_Desc e2 = {V_ATTRIBUTE_TYPE_TEXTURE_COORD, FORMAT_R32G32_FLOAT, V_ATTRIBUTE_FREQ_PER_VERTEX};
	layout.elements = {e0, e1, e2};
	auto prog = create_program(
		vs,
		ps,
		layout,
		plane.verts.data(),
		plane.verts.size() * sizeof(Vertex_attribute),
		sizeof(Vertex_attribute),
		0,
		plane.verts.size());

	auto pass = create_render_pass(prog, main_rt, L"pass - render ground plane", clear_color);
	set_drawing_mode(pass, DRAWING_MODE_TRIANGLES);
}

void RenderEngine::_render_lights()
{
	for (auto& light: state.scene.pLights)
	{
		auto mats = new opaque_pass_mats_unifrom; // TODO(adel): fix leak
		mats->model_world = glm::identity<glm::mat4>();
		mats->world_camera = m_geometry->world_camera_transform;
		mats->camera_ndc = m_geometry->camera_ndc_transform;

		auto vs_uniform_mat = create_uniform("mats", mats, sizeof(opaque_pass_mats_unifrom), 0);
		std::vector<Uniform> vs_uniforms = {vs_uniform_mat};
		std::vector<Texture> vs_textures = {};

		auto vs_update = [this, mats]()
		{
			opaque_pass_mats_unifrom new_mats{};
			new_mats.model_world = glm::identity<glm::mat4>();
			new_mats.world_camera = m_geometry->world_camera_transform;
			new_mats.camera_ndc = m_geometry->camera_ndc_transform;
			memcpy(mats, &new_mats, sizeof(opaque_pass_mats_unifrom));
		};

		auto vs = create_shader(
			L"../../assets/shaders/wireframe.hlsl",
			"vs_main",
			SHADER_STAGE_VERTEX,
			vs_uniforms,
			vs_textures,
			vs_update);

		std::vector<Uniform> ps_uniforms = {};
		std::vector<Texture> ps_textures = {};
		auto ps_update = [](){};

		auto ps = create_shader(
			L"../../assets/shaders/wireframe.hlsl",
			"ps_main",
			SHADER_STAGE_PIXEL,
			ps_uniforms,
			ps_textures,
			ps_update);

		Input_Layout layout{};
		Element_Desc e0 = {V_ATTRIBUTE_TYPE_POSITION, FORMAT_R32G32B32A32_FLOAT, V_ATTRIBUTE_FREQ_PER_VERTEX};
		layout.elements = {e0};

		Bounding_Box light_bb{
			.min_x = -100 + light.position.x,
			.min_y = -100 + light.position.y,
			.min_z = -100 + light.position.z,
			.max_x = 100 + light.position.x,
			.max_y = 100 + light.position.y,
			.max_z = 100 + light.position.z,
		};
		auto bb_verts = _bounding_box_lines(light_bb);

		auto _prog = create_program(
			vs,
			ps,
			layout,
			bb_verts->data(),
			bb_verts->size() * sizeof(glm::vec4),
			sizeof(glm::vec4),
			0,
			bb_verts->size());

		auto pass = create_render_pass(_prog, main_rt, L"pass - render lights", clear_color);
		set_drawing_mode(pass, DRAWING_MODE_LINES);
	}
}

void RenderEngine::_render_opaques()
{
	auto &plight = state.scene.pLights[0]; // TODO(adel): account for multiple light sources in the scene
	main_rt = create_render_target(
		"",
		state.window.width,
		state.window.height,
		state.window.bytes_per_pixel);

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

		auto vs_update = [this, &model, mats]()
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

		auto pass = create_render_pass(_prog, main_rt, L"pass - render opaques", clear_color);
		set_drawing_mode(pass, DRAWING_MODE_TRIANGLES);
	}
}

void RenderEngine::_render_opaques_reflected()
{
	auto &plight = state.scene.pLights[0]; // TODO(adel): account for multiple light sources in the scene
	mirrored_scene_rt = create_render_target(
		"",
		state.window.width,
		state.window.height,
		state.window.bytes_per_pixel);

	size_t model_id = 0;
	for (auto& model: state.scene.models)
	{
		auto mats = new opaque_pass_mats_unifrom; // TODO(adel): fix leak
		// mats->model_world =glm::translate(glm::scale(m_geometry->model_world_transform, glm::vec3(1, -1, 1)), glm::vec3(0, 3, 0));
		mats->model_world = glm::scale(m_geometry->model_world_transform, glm::vec3(1, -1, 1));
		mats->world_camera = m_geometry->world_camera_transform;
		mats->camera_ndc = m_geometry->camera_ndc_transform;

		auto vs_uniform_mat = create_uniform("mats", mats, sizeof(opaque_pass_mats_unifrom), 0);
		auto vs_uniform_light = create_uniform("light", &plight, sizeof(PointLight), 1);
		std::vector<Uniform> vs_uniforms = {vs_uniform_mat, vs_uniform_light};
		std::vector<Texture> vs_textures = {};

		auto vs_update = [this, &model, mats]()
		{
			opaque_pass_mats_unifrom new_mats{};
			new_mats.model_world = glm::scale(m_geometry->model_world_transform * model.model_world_transfrom, glm::vec3(1, -1, 1));
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

		auto pass = create_render_pass(_prog, mirrored_scene_rt, L"pass - render opaques mirrored", glm::vec4(1,1,1,0));
		set_drawing_mode(pass, DRAWING_MODE_TRIANGLES);
	}
}

struct _skybox_pass_uniform
{
	glm::mat4 NDCWorld;
};

void RenderEngine::_render_skybox()
{
	auto mat = new _skybox_pass_uniform;
	mat->NDCWorld = glm::inverse(m_geometry->model_world_transform) * glm::inverse(m_geometry->camera_ndc_transform);

	auto vs_uniform_mat = create_uniform("mats", mat, sizeof(_skybox_pass_uniform), 0);
	std::vector<Uniform> vs_uniforms = {vs_uniform_mat};
	std::vector<Texture> vs_textures = {};

	auto vs_update = [this, mat]()
	{
		_skybox_pass_uniform new_mats{};
		new_mats.NDCWorld = glm::inverse(m_geometry->model_world_transform) * glm::inverse(m_geometry->camera_ndc_transform);
		memcpy(mat, &new_mats, sizeof(_skybox_pass_uniform));
	};

	auto vs = create_shader(
		L"../../assets/shaders/envMap.hlsl",
		"vs_main",
		SHADER_STAGE_VERTEX,
		vs_uniforms,
		vs_textures,
		vs_update);

	char *cube_data[6] = {
		state.scene.skybox.right.data[0],
		state.scene.skybox.left.data[0],
		state.scene.skybox.top.data[0],
		state.scene.skybox.bottom.data[0],
		state.scene.skybox.front.data[0],
		state.scene.skybox.back.data[0],
	};
	auto ps_t = create_texture(
		"skybox_pass_t",
		Texture::DIM_CUBE,
		cube_data,
		state.scene.skybox.back.width, state.scene.skybox.back.height,
		state.scene.skybox.back.bytes_per_pixel,
		state.scene.skybox.back.height * state.scene.skybox.back.width * state.scene.skybox.back.bytes_per_pixel,
		0);

	std::vector<Uniform> ps_uniforms = {};
	std::vector<Texture> ps_textures = {ps_t};

	auto ps_update = [](){};

	auto ps = create_shader(
		L"../../assets/shaders/envMap.hlsl",
		"ps_main",
		SHADER_STAGE_PIXEL,
		ps_uniforms,
		ps_textures,
		ps_update);

	Input_Layout layout{};
	auto prog = create_program(
		vs,
		ps,
		layout,
		nullptr,
		0,
		0, 0, 6);

	auto skybox_pass = create_render_pass(prog, main_rt, L"pass - render skybox", clear_color);
	set_drawing_mode(skybox_pass, DRAWING_MODE_TRIANGLES);
}

void RenderEngine::_render_shadows()
{
	auto &plight = state.scene.pLights[0]; // TODO(adel): account for multiple light sources in the scene
	depth_rt = create_render_target(
		"",
		state.window.width,
		state.window.height,
		state.window.bytes_per_pixel);

	auto scene_width = state.scene.bb.max_x - state.scene.bb.min_x;
	auto scene_height = state.scene.bb.max_y - state.scene.bb.min_y;
	auto scene_depth = state.scene.bb.max_z - state.scene.bb.min_z;

	auto scene_center = glm::vec3(
		state.scene.bb.min_x + scene_width / 2.0,
		state.scene.bb.min_y,
		state.scene.bb.min_z + scene_depth / 2.0);

	for (auto &model : state.scene.models)
	{
		if (model.cast_shadow == false)
			continue;

		auto scene_center_ws = m_geometry->model_world_transform * glm::vec4(scene_center, 1);
		auto light_dir = glm::vec3(scene_center_ws) - plight.position;
		float fovy = atan2f(state.view_volume.top_plane, state.view_volume.near_plane) * 2;

		auto light_mats = new opaque_pass_mats_unifrom;
		light_mats->model_world = m_geometry->model_world_transform;
		light_mats->world_camera = glm::lookAtLH(plight.position, light_dir, glm::vec3(0, 1, 0));
		light_mats->camera_ndc = glm::perspectiveLH(fovy, (float)state.window.width / state.window.height, state.view_volume.near_plane, state.view_volume.far_plane);

		auto vs_uniform_mat = create_uniform("mats", light_mats, sizeof(opaque_pass_mats_unifrom), 0);
		std::vector<Uniform> vs_uniforms = {vs_uniform_mat};
		std::vector<Texture> vs_textures = {};

		auto vs_update = [this, light_mats, plight, fovy, scene_center]()
		{
			auto scene_center_ws = m_geometry->model_world_transform * glm::vec4(scene_center, 1);
			auto light_dir = glm::vec3(scene_center_ws) - plight.position;

			opaque_pass_mats_unifrom new_mats{};
			new_mats.model_world = m_geometry->model_world_transform;
			new_mats.world_camera = glm::lookAtLH(plight.position, light_dir, glm::vec3(0, 1, 0));
			new_mats.camera_ndc = glm::perspectiveLH(fovy, (float)state.window.width / state.window.height, state.view_volume.near_plane, state.view_volume.far_plane);

			memcpy(light_mats, &new_mats, sizeof(opaque_pass_mats_unifrom));
		};

		auto vs = create_shader(
			L"../../assets/shaders/depth.hlsl",
			"vs_main",
			SHADER_STAGE_VERTEX,
			vs_uniforms,
			vs_textures,
			vs_update);

		std::vector<Uniform> ps_uniforms = {};
		std::vector<Texture> ps_textures = {};
		auto ps_update = []() {};

		auto ps = create_shader(
			L"../../assets/shaders/depth.hlsl",
			"ps_main",
			SHADER_STAGE_PIXEL,
			ps_uniforms,
			ps_textures,
			ps_update);

		auto prog = create_program(
			vs,
			ps,
			model.layout,
			model.verts.data(),
			model.verts.size() * sizeof(Vertex_attribute),
			sizeof(Vertex_attribute),
			0,
			model.verts.size());

		auto pass = create_render_pass(prog, depth_rt, L"pass - render light view depth", clear_color);
		set_drawing_mode(pass, DRAWING_MODE_TRIANGLES);
	}
}

void RenderEngine::scene_finish()
{
	// opqaue pass
	_render_opaques();
	_gen_scene_bounding_box();
	if (options.render_bounding_boxes)
	{
		_render_bounding_boxes();
	}
	if (options.render_lights)
	{
		_render_lights();
	}
	if (options.render_shadows)
	{
		_render_shadows();
	}
	if (options.render_ground)
	{
		if (options.ground_is_mirror)
		{
			_render_opaques_reflected();
		}
		_render_ground();
	}
	if (options.render_skybox)
	{
		_render_skybox();
	}
}

void RenderEngine::render_bounding_boxes(bool on)
{
	options.render_bounding_boxes = on;
}

void RenderEngine::render_lights(bool on)
{
	options.render_lights = on;
}

void RenderEngine::render_ground(bool on)
{
	options.render_ground = on;
}

void RenderEngine::mirror_ground(bool on)
{
	options.ground_is_mirror = on;
}

void RenderEngine::render_skybox(bool on)
{
	options.render_skybox &= on;
}

void RenderEngine::render_shadows(bool on)
{
	options.render_shadows = on;
}

void RenderEngine::set_clear_color(glm::vec4 color)
{
	this->clear_color = color;
}

void RenderEngine::_gen_scene_bounding_box()
{
	state.scene.bb = {
		.min_x = std::numeric_limits<float>::infinity(),
		.min_y = std::numeric_limits<float>::infinity(),
		.min_z = std::numeric_limits<float>::infinity(),
		.max_x = -std::numeric_limits<float>::infinity(),
		.max_y = -std::numeric_limits<float>::infinity(),
		.max_z = -std::numeric_limits<float>::infinity()
	};

	for (auto model: state.scene.models)
	{
		state.scene.bb.min_x = std::min(state.scene.bb.min_x, model.bb.min_x);
		state.scene.bb.min_y = std::min(state.scene.bb.min_y, model.bb.min_y);
		state.scene.bb.min_z = std::min(state.scene.bb.min_z, model.bb.min_z);

		state.scene.bb.max_x = std::max(state.scene.bb.max_x, model.bb.max_x);
		state.scene.bb.max_y = std::max(state.scene.bb.max_y, model.bb.max_y);
		state.scene.bb.max_z = std::max(state.scene.bb.max_z, model.bb.max_z);
	}
}