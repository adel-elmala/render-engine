#pragma once 

#include <glm/glm.hpp>
#include "common.h"

class Geometry
{
public:
	Geometry();
	~Geometry();

	void bind_state(Engine_State* engine_state) { if (engine_state) state = engine_state; }
	glm::vec4 transform_model_to_window(glm::vec3 v_model_space);
	void update_viewport_transform();
	void update_camera_transform();
	void update_perspective_transform();
	void update_world_transform();

private:
	glm::mat4 model_world_transform;
	glm::mat4 world_camera_transform;
	glm::mat4 camera_ndc_transform;
	glm::mat4 ndc_window_transform;

	Engine_State* state;
};
