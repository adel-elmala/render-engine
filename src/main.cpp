#include <cstdlib>

#include "../include/renderEngine.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp> // translate, rotate, scale, perspective

struct _pass_0_mats
{
	glm::mat4 model_world;
	glm::mat4 world_camera;
	glm::mat4 camera_ndc;
};

struct _pass_1_mat
{
	glm::mat4 NDCWorld;
};

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

	engine.render_bounding_boxes(true);
	engine.render_lights(true);
	engine.render_ground(true);
	engine.scene_finish();
	// TODO(adel): tell the engine where is the view-volume (optionally)

	// TODO(adel): mark the ground-plane as reflected, so do this reflection pass internally
	// // pass 1 - render refelcted bunny to texture
	// {
	// 	auto prev_pass = engine.passes[0];
	// 	auto pass_1_render_target = engine.create_render_target(
	// 		"pass_1",
	// 		engine.state.window.width,
	// 		engine.state.window.height,
	// 		engine.state.window.bytes_per_pixel);
	// 	auto pass_1 = engine.create_render_pass(prev_pass.used_prog, pass_1_render_target, L"pass 1 - render reflected bunny");
	// }

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

	// // TODO(adel): use the scene bounding box, and infer where to render this plane, and also do this pass internally
	

	// // TODO(adel): optioanlly render the skybox throung an  option to the engine
	// // pass 4 - render skybox
	// {
	// 	_pass_1_mat mat{};
	// 	mat.NDCWorld = glm::inverse(engine.m_geometry->model_world_transform) * glm::inverse(engine.m_geometry->camera_ndc_transform);

	// 	auto skybox_pass_vs_uniform_mat = engine.create_uniform("mats", &mat, sizeof(mat), 0);
	// 	std::vector<Uniform> skybox_pass_vs_uniforms = {skybox_pass_vs_uniform_mat};
	// 	std::vector<Texture> skybox_pass_vs_textures = {};

	// 	auto skybox_pass_vs = engine.create_shader(
	// 		L"../../assets/shaders/envMap.hlsl",
	// 		"vs_main",
	// 		SHADER_STAGE_VERTEX,
	// 		skybox_pass_vs_uniforms,
	// 		skybox_pass_vs_textures);

	// 	auto env_map = engine.m_scene_manager->load_env_texture_cube("../../assets/models/skybox/");
	// 	char *cube_data[6] = {
	// 		env_map.right.data[0],
	// 		env_map.left.data[0],
	// 		env_map.top.data[0],
	// 		env_map.bottom.data[0],
	// 		env_map.front.data[0],
	// 		env_map.back.data[0],
	// 	};
	// 	auto skybox_pass_ps_t = engine.create_texture(
	// 		"skybox_pass_t",
	// 		Texture::DIM_CUBE,
	// 		cube_data,
	// 		env_map.back.width, env_map.back.height,
	// 		env_map.back.bytes_per_pixel,
	// 		env_map.back.height * env_map.back.width * env_map.back.bytes_per_pixel,
	// 		0);

	// 	std::vector<Uniform> skybox_pass_ps_uniforms = {};
	// 	std::vector<Texture> skybox_pass_ps_textures = {skybox_pass_ps_t};
	// 	auto skybox_pass_ps = engine.create_shader(
	// 		L"../../assets/shaders/envMap.hlsl",
	// 		"ps_main",
	// 		SHADER_STAGE_PIXEL,
	// 		skybox_pass_ps_uniforms,
	// 		skybox_pass_ps_textures);

	// 	Input_Layout layout{};
	// 	auto skybox_pass_prog = engine.create_program(
	// 		skybox_pass_vs,
	// 		skybox_pass_ps,
	// 		layout,
	// 		nullptr,
	// 		0,
	// 		0, 0, 6);

	// 	auto skybox_pass_render_target = engine.passes[0].render_target;
	// 	auto skybox_pass = engine.create_render_pass(skybox_pass_prog, skybox_pass_render_target, L"pass - render skybox");
	// }

	while (engine.should_exit() == false)
	{
		// NOTE(adel): ALL of the uniforms updates should be internal to the engine ? 
		// update uniforms
		// engine.m_geometry->update_world_transform();
		// engine.m_geometry->update_camera_transform();
		// engine.m_geometry->update_perspective_transform();

		// _pass_0_mats mats{};
		// mats.model_world = engine.m_geometry->model_world_transform;
		// mats.world_camera = engine.m_geometry->world_camera_transform;
		// mats.camera_ndc = engine.m_geometry->camera_ndc_transform;
		// engine.passes[0].used_prog.vs.uniforms[0].data = &mats;
		
		// _pass_0_mats mats_2{};
		// mats_2.model_world =glm::translate(glm::scale(engine.m_geometry->model_world_transform, glm::vec3(1, -1, 1)), glm::vec3(0, 3, 0));
		// mats_2.world_camera = engine.m_geometry->world_camera_transform;
		// mats_2.camera_ndc = engine.m_geometry->camera_ndc_transform;
		// engine.passes[1].used_prog.vs.uniforms[0].data = &mats_2;

		// _pass_0_mats depth_mats{};
		// depth_mats.model_world = engine.m_geometry->model_world_transform;
		// auto light_dir = glm::vec3{0, 0, (engine.state.view_volume.near_plane + ((engine.state.view_volume.far_plane - engine.state.view_volume.near_plane) / 2))} - light.position;
		// depth_mats.world_camera = glm::lookAtLH(light.position, light_dir, glm::vec3(0, 1, 0));
		// float fovy = atan2f(engine.state.view_volume.top_plane, engine.state.view_volume.near_plane) * 2;
		// depth_mats.camera_ndc = glm::perspectiveLH(fovy, (float)engine.state.window.width / engine.state.window.height, engine.state.view_volume.near_plane, engine.state.view_volume.far_plane);
		// engine.passes[2].used_prog.vs.uniforms[0].data = &depth_mats;

		// engine.passes[3].used_prog.vs.uniforms[0].data = &mats;
		// engine.passes[3].used_prog.vs.uniforms[1].data = &depth_mats;


		// _pass_1_mat mat{};
		// mat.NDCWorld = glm::inverse(engine.m_geometry->model_world_transform) * glm::inverse(engine.m_geometry->camera_ndc_transform);
		// engine.passes[4].used_prog.vs.uniforms[0].data = &mat;

		engine.render_frame();
		engine.m_win_manager->start_event_loop();
		engine.flush_frame();
	}

	return EXIT_SUCCESS;
}