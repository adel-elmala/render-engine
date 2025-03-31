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
	RenderEngine engine(BACKEND_D3D11, "../../assets/bunny/bunny.obj");

	// pass 0 - render bunny to texture
	{
		_pass_0_mats mats{};
		mats.model_world = engine.m_geometry->model_world_transform;
		mats.world_camera = engine.m_geometry->world_camera_transform;
		mats.camera_ndc = engine.m_geometry->camera_ndc_transform;

		Material mtl{};
		mtl.ka = engine.state.m_model.mtl.ka;
		mtl.kd = engine.state.m_model.mtl.kd;
		mtl.ks = engine.state.m_model.mtl.ks;
		mtl.ns = engine.state.m_model.mtl.ns;

		PointLight light{};
		light.position = glm::vec3(100.0f, 100.0f, 100.0f);						// in world space
		light.color = glm::vec3(242.0 / 255.0f, 196.0 / 255.0f, 29.0 / 255.0f); // yellowish;
		light.intensity = 4.0f;

		auto pass_0_vs_uniform_mat = engine.create_uniform("mats", &mats, sizeof(mats), 0);
		auto pass_0_vs_uniform_light = engine.create_uniform("light", &light, sizeof(light), 1);
		auto pass_0_vs_uniform_mtl = engine.create_uniform("mtl", &mtl, sizeof(mtl), 2);
		std::vector<Uniform> pass_0_vs_uniforms = {pass_0_vs_uniform_mat, pass_0_vs_uniform_light, pass_0_vs_uniform_mtl};
		std::vector<Texture> pass_0_vs_textures = {};

		auto pass_0_vs = engine.create_shader(
			L"../../assets/shaders/shaders.hlsl",
			"vs_main",
			SHADER_STAGE_VERTEX,
			pass_0_vs_uniforms,
			pass_0_vs_textures);

		auto model_texture = engine.state.m_model.m_gpu.textures[0];
		auto pass_0_ps_t = engine.create_texture(
			"t",
			Texture::DIM_2D,
			model_texture.data,
			model_texture.width, model_texture.height,
			model_texture.bytes_per_pixel,
			model_texture.height * model_texture.width * model_texture.bytes_per_pixel,
			0);

		std::vector<Uniform> pass_0_ps_uniforms = {};
		std::vector<Texture> pass_0_ps_textures = {pass_0_ps_t};
		auto pass_0_ps = engine.create_shader(
			L"../../assets/shaders/shaders.hlsl",
			"ps_main",
			SHADER_STAGE_PIXEL,
			pass_0_ps_uniforms,
			pass_0_ps_textures);

		Input_Layout layout{};
		Element_Desc e0 = {V_ATTRIBUTE_TYPE_POSITION, FORMAT_R32G32B32A32_FLOAT, V_ATTRIBUTE_FREQ_PER_VERTEX};
		Element_Desc e1 = {V_ATTRIBUTE_TYPE_NORMAL, FORMAT_R32G32B32A32_FLOAT, V_ATTRIBUTE_FREQ_PER_VERTEX};
		Element_Desc e2 = {V_ATTRIBUTE_TYPE_TEXTURE_COORD, FORMAT_R32G32_FLOAT, V_ATTRIBUTE_FREQ_PER_VERTEX};
		layout.elements = {e0, e1, e2};
		auto pass_0_prog = engine.create_program(
			pass_0_vs,
			pass_0_ps,
			layout,
			engine.state.m_model.m_gpu.verts.data(),
			engine.state.m_model.m_gpu.verts.size() * sizeof(Vertex_attribute),
			sizeof(Vertex_attribute),
			0,
			engine.state.m_model.m_gpu.verts.size());

		auto pass_0_render_target = engine.create_render_target(
			"pass_0",
			engine.state.m_window.width,
			engine.state.m_window.height,
			engine.state.m_window.bytes_per_pixel);

		auto pass_0 = engine.create_render_pass(pass_0_prog, pass_0_render_target);
	}

	// pass 1 - render refelcted bunny to texture
	{
		auto prev_pass = engine.passes.back();
		auto pass_1 = engine.create_render_pass(prev_pass.used_prog, engine.passes.back().render_target);
	}

	// pass 2 - render skybox
	{
		_pass_1_mat mat{};
		mat.NDCWorld = glm::inverse(engine.m_geometry->model_world_transform) * glm::inverse(engine.m_geometry->camera_ndc_transform);

		auto pass_1_vs_uniform_mat = engine.create_uniform("mats", &mat, sizeof(mat), 0);
		std::vector<Uniform> pass_1_vs_uniforms = {pass_1_vs_uniform_mat};
		std::vector<Texture> pass_1_vs_textures = {};

		auto pass_1_vs = engine.create_shader(
			L"../../assets/shaders/envMap.hlsl",
			"vs_main",
			SHADER_STAGE_VERTEX,
			pass_1_vs_uniforms,
			pass_1_vs_textures);

		// TODO(adel): handle cube texture - engine side
		auto env_map = engine.state.m_model.env_map;
		char *cube_data[6] = {
			env_map.right.data[0],
			env_map.left.data[0],
			env_map.top.data[0],
			env_map.bottom.data[0],
			env_map.front.data[0],
			env_map.back.data[0],
		};
		auto pass_1_ps_t = engine.create_texture(
			"t",
			Texture::DIM_CUBE,
			cube_data,
			env_map.back.width, env_map.back.height,
			env_map.back.bytes_per_pixel,
			env_map.back.height * env_map.back.width * env_map.back.bytes_per_pixel,
			0);

		std::vector<Uniform> pass_1_ps_uniforms = {};
		std::vector<Texture> pass_1_ps_textures = {pass_1_ps_t};
		auto pass_1_ps = engine.create_shader(
			L"../../assets/shaders/envMap.hlsl",
			"ps_main",
			SHADER_STAGE_PIXEL,
			pass_1_ps_uniforms,
			pass_1_ps_textures);

		Input_Layout layout{};
		auto pass_1_prog = engine.create_program(
			pass_1_vs,
			pass_1_ps,
			layout,
			nullptr,
			0,
			0, 0, 6);

		auto pass_1_render_target = engine.passes.back().render_target;
		auto pass_1 = engine.create_render_pass(pass_1_prog, pass_1_render_target);
	}

	while (engine.should_exit() == false)
	{
		// update uniforms
		engine.m_geometry->update_world_transform();
		engine.m_geometry->update_camera_transform();
		engine.m_geometry->update_perspective_transform();

		_pass_0_mats mats{};
		mats.model_world = engine.m_geometry->model_world_transform;
		mats.world_camera = engine.m_geometry->world_camera_transform;
		mats.camera_ndc = engine.m_geometry->camera_ndc_transform;
		engine.passes[0].used_prog.vs.uniforms[0].data = &mats;
		
		_pass_0_mats mats_2{};
		mats_2.model_world =glm::translate(glm::scale(engine.m_geometry->model_world_transform, glm::vec3(1, -1, 1)), glm::vec3(0,500 ,0));;
		mats_2.world_camera = engine.m_geometry->world_camera_transform;
		mats_2.camera_ndc = engine.m_geometry->camera_ndc_transform;
		engine.passes[1].used_prog.vs.uniforms[0].data = &mats_2;
		
		_pass_1_mat mat{};
		mat.NDCWorld = glm::inverse(engine.m_geometry->model_world_transform) * glm::inverse(engine.m_geometry->camera_ndc_transform);
		engine.passes[2].used_prog.vs.uniforms[0].data = &mat;

		engine.render_frame();
		engine.m_win_manager->start_event_loop();
		engine.flush_frame();
	}

	return EXIT_SUCCESS;
}