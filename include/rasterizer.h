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
	void draw_triangles();
	void draw_point(glm::vec3& point, glm::u8vec4& color);
	void draw_line(glm::vec3& p1, glm::vec3& p2, glm::u8vec4& color);
	void draw_triangle(Face& triangle);
	//glm::vec3 barycentric_coords(Face& triangle, glm::vec2& candidate_pixel);
	std::pair<glm::ivec2, glm::ivec2> triangle_bounding_box(const glm::ivec2& p0, const  glm::ivec2& p1, const  glm::ivec2& p2);
	glm::u8vec4 sample_texture(Texture& t, glm::vec2 uv);

	Engine_State* state;
};

__forceinline float implicit_2d_line_eq(glm::vec2 line_left_p, glm::vec2 line_right_p, glm::vec2 p);