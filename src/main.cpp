#include <cstdlib>

#include "../include/renderEngine.h"
#include <glm/glm.hpp>

int main(int argc, char **argv)
{
	RenderEngine engine(BACKEND_D3D11);
	auto bunny = engine.m_scene_manager->parse_model("../../assets/models/bunny/vbunny.obj");

	Input_Layout layout{};
	Element_Desc e0 = {V_ATTRIBUTE_TYPE_POSITION, FORMAT_R32G32B32A32_FLOAT, V_ATTRIBUTE_FREQ_PER_VERTEX};
	Element_Desc e1 = {V_ATTRIBUTE_TYPE_NORMAL, FORMAT_R32G32B32A32_FLOAT, V_ATTRIBUTE_FREQ_PER_VERTEX};
	Element_Desc e2 = {V_ATTRIBUTE_TYPE_TEXTURE_COORD, FORMAT_R32G32_FLOAT, V_ATTRIBUTE_FREQ_PER_VERTEX};
	layout.elements = {e0, e1, e2};
	bunny.layout = layout;

	engine.scene_add_model(bunny);

	PointLight l0{}; // TODO(adel): make lights part of the scene description that the engine has control over
	l0.position = glm::vec3(0.0f, 300.0f, 1300.0f);						// in world space
	l0.color = glm::vec3(242.0 / 255.0f, 196.0 / 255.0f, 29.0 / 255.0f); // yellowish;
	l0.intensity = 4.0f;
	engine.scene_add_point_light(l0);

	Camera cam{};
	cam.position = glm::vec3{0.0f, 0.0f, 0.0f};
	cam.lookat = glm::vec3{0.0f, 0.0f, 1.0f};
	cam.up = glm::vec3{0.0f, 1.0f, 0.0f};
	cam.sensitivity = 3.5f;
	engine.scene_update_camera(cam);

	auto skybox = engine.m_scene_manager->load_env_texture_cube("../../assets/models/skybox/");
	engine.scene_add_skybox(skybox);

	engine.render_bounding_boxes(true);
	engine.render_lights(true);
	engine.render_ground(true);
	engine.render_skybox(true);
	engine.mirror_ground(true);
	engine.set_clear_color(glm::vec4(0.1, 0.2, 0.6, 1.0));
	engine.scene_finish();
	// TODO(adel): tell the engine where is the view-volume (optionally)

	// // TODO(adel): mark the models if it casts shadow or not, and do this pass internally
	// // depth pass - render light camera's depth map
	// {
	// 	_pass_0_mats mats{};
	// 	mats.model_world = engine.m_geometry->model_world_transform;
	// 	auto light_dir = glm::vec3{0, 0, (engine.state.view_volume.near_plane + ((engine.state.view_volume.far_plane - engine.state.view_volume.near_plane) / 2))} - light.position;
	// 	mats.world_camera = glm::lookAtLH(light.position, light_dir, glm::vec3(0, 1, 0));
	// 	float fovy = atan2f(engine.state.view_volume.top_plane, engine.state.view_volume.near_plane) * 2;
	// 	mats.camera_ndc = glm::perspectiveLH(fovy, (float)engine.state.window.width / engine.state.window.height, engine.state.view_volume.near_plane, engine.state.view_volume.far_plane);

	// 	auto depth_pass_vs_uniform_mat = engine.create_uniform("mats", &mats, sizeof(mats), 0);
	// 	auto depth_pass_vs_uniform_light = engine.create_uniform("light", &light, sizeof(light), 1);
	// 	std::vector<Uniform> depth_pass_vs_uniforms = {depth_pass_vs_uniform_mat, depth_pass_vs_uniform_light};
	// 	std::vector<Texture> depth_pass_vs_textures = {};

	// 	auto depth_pass_vs = engine.create_shader(
	// 		L"../../assets/shaders/depth.hlsl",
	// 		"vs_main",
	// 		SHADER_STAGE_VERTEX,
	// 		depth_pass_vs_uniforms,
	// 		depth_pass_vs_textures);

	// 	std::vector<Uniform> depth_pass_ps_uniforms = {};
	// 	std::vector<Texture> depth_pass_ps_textures = {};
	// 	auto depth_pass_ps = engine.create_shader(
	// 		L"../../assets/shaders/depth.hlsl",
	// 		"ps_main",
	// 		SHADER_STAGE_PIXEL,
	// 		depth_pass_ps_uniforms,
	// 		depth_pass_ps_textures);

	// 	Input_Layout layout{};
	// 	Element_Desc e0 = {V_ATTRIBUTE_TYPE_POSITION, FORMAT_R32G32B32A32_FLOAT, V_ATTRIBUTE_FREQ_PER_VERTEX};
	// 	Element_Desc e1 = {V_ATTRIBUTE_TYPE_NORMAL, FORMAT_R32G32B32A32_FLOAT, V_ATTRIBUTE_FREQ_PER_VERTEX};
	// 	Element_Desc e2 = {V_ATTRIBUTE_TYPE_TEXTURE_COORD, FORMAT_R32G32_FLOAT, V_ATTRIBUTE_FREQ_PER_VERTEX};
	// 	layout.elements = {e0, e1, e2};
	// 	auto depth_pass_prog = engine.create_program(
	// 		depth_pass_vs,
	// 		depth_pass_ps,
	// 		layout,
	// 		bunny.verts.data(),
	// 		bunny.verts.size() * sizeof(Vertex_attribute),
	// 		sizeof(Vertex_attribute),
	// 		0,
	// 		bunny.verts.size());

	// 	auto depth_pass_render_target = engine.create_render_target(
	// 		"depth_pass",
	// 		engine.state.window.width,
	// 		engine.state.window.height,
	// 		engine.state.window.bytes_per_pixel);

	// 	auto depth_pass = engine.create_render_pass(depth_pass_prog, depth_pass_render_target, L"pass - render light camera depth");
	// }

	while (engine.should_exit() == false)
	{
		// _pass_0_mats depth_mats{};
		// depth_mats.model_world = engine.m_geometry->model_world_transform;
		// auto light_dir = glm::vec3{0, 0, (engine.state.view_volume.near_plane + ((engine.state.view_volume.far_plane - engine.state.view_volume.near_plane) / 2))} - light.position;
		// depth_mats.world_camera = glm::lookAtLH(light.position, light_dir, glm::vec3(0, 1, 0));
		// float fovy = atan2f(engine.state.view_volume.top_plane, engine.state.view_volume.near_plane) * 2;
		// depth_mats.camera_ndc = glm::perspectiveLH(fovy, (float)engine.state.window.width / engine.state.window.height, engine.state.view_volume.near_plane, engine.state.view_volume.far_plane);
		// engine.passes[2].used_prog.vs.uniforms[0].data = &depth_mats;

		// engine.passes[3].used_prog.vs.uniforms[0].data = &mats;
		// engine.passes[3].used_prog.vs.uniforms[1].data = &depth_mats;
		engine.render_frame();
		engine.m_win_manager->start_event_loop();
		engine.flush_frame();
	}

	return EXIT_SUCCESS;
}