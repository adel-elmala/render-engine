#pragma once

#include "common.h"

class Rasterizer
{
public:
	Rasterizer();
	~Rasterizer();
	void bind_state(Engine_State* engine_state) { if (engine_state) state = engine_state; }
	void run();
private:
	void draw_points();
	void draw_lines();
	void draw_point(glm::ivec2 point, glm::vec4 color);
	void draw_line(glm::vec2 p1, glm::vec2 p2, glm::vec4 color);
	Engine_State* state;

};

