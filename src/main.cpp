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
	bunny.cast_shadow = true;

	engine.scene_add_model(bunny);

	PointLight l0{};
	// l0.position = glm::vec3(0.0f, 100.0f, 700.0f);						// in world space
	l0.position = glm::vec3(0.0f, 0.0f, 0.0f);						// in world space
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
	engine.render_shadows(true);
	engine.mirror_ground(true);
	engine.set_clear_color(glm::vec4(0.1, 0.2, 0.6, 1.0));
	engine.scene_finish();
	// TODO(adel): tell the engine where is the view-volume (optionally)

	while (engine.should_exit() == false)
	{
		engine.render_frame();
		engine.m_win_manager->start_event_loop();
		engine.flush_frame();
	}

	return EXIT_SUCCESS;
}