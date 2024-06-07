#pragma once 

#include "common.h"

class Geometry
{
public:
	Geometry();
	~Geometry();

	void bind_state(Engine_State* engine_state) { if (engine_state) state = engine_state; }
	void run();

private:
	
	void lighting_calc();
	void clipping();
	void clip_triangles();
	bool in_view_volume(glm::vec4& point);

	void send_to_camera_space();
	void send_to_ndc_space();
	void send_to_pixel_space();

	void update_viewport_transform();
	void update_camera_transform();
	void update_perspective_transform();
	void update_world_transform();

	glm::mat4 model_world_transform;
	glm::mat4 world_camera_transform;
	glm::mat4 camera_ndc_transform;
	glm::mat4 ndc_pixel_transform;

	Engine_State* state;
};
