#include "../include/rasterizer.h"


Rasterizer::Rasterizer()
{
}

Rasterizer::~Rasterizer()
{
}

void Rasterizer::run()
{
	draw_lines();
}

void Rasterizer::draw_points()
{
	std::unique_lock lock(state->m_swapchain.m);

	int count = 0;
	// TODO(adel) : should i lock the swapchain here instead ?
	for (auto face : state->m_model.faces)
	{
		auto p0 = state->m_model.positions[face.p_indices.x];
		auto p1 = state->m_model.positions[face.p_indices.y];
		auto p2 = state->m_model.positions[face.p_indices.z];

		//auto c0 = state->m_model.colors[face.[0]];
		//auto c1 = state->m_model.colors[face.c_indices[1]];
		//auto c2 = state->m_model.colors[face.c_indices[2]];
		draw_point(p0, glm::vec4{ 0xff,0x00,0x00,0xff });
		draw_point(p1, glm::vec4{ 0xff,0x00,0x00,0xff });
		draw_point(p2, glm::vec4{ 0xff,0x00,0x00,0xff });
		count++;
	}
}
void Rasterizer::draw_point(glm::ivec2 point, glm::vec4 color)
{
	// TODO[adel] remove this and add clipping
	if (point.x >= state->m_swapchain.frame_width || point.x < 0 || point.y >= state->m_swapchain.frame_height || point.y < 0)
		return;

	// pixel memory layout BB GG RR AA
	char* start = (char*)state->m_swapchain.back_buffer +
		(point.x * state->m_swapchain.frame_bytes_per_pixel) +
		(point.y * state->m_swapchain.frame_width * state->m_swapchain.frame_bytes_per_pixel);
	switch (state->m_swapchain.frame_bytes_per_pixel)
	{
	case 1:
		*start = 0x00;
		break;
	case 2:
		*(uint16_t*)start = 0x00ff;
		break;
	case 3:
		*(uint16_t*)start = 0x0000;
		start += 2;
		*start = 0xff;
		break;
	case 4:
		*(uint32_t*)start = 0xffffffff;
		break;
	}

}
float implicit_2d_line_eq(glm::vec2 line_left_p, glm::vec2 line_right_p, glm::vec2 p)
{
	// p0 left  --  p1 right
	// f(x,y) = (y0 −y1)x +(x1 −x0)y +x0y1 −x1y0
	auto c0 = line_left_p.y - line_right_p.y;
	auto c1 = line_right_p.x - line_left_p.x;
	auto c2 = (line_left_p.x * line_right_p.y) - (line_right_p.x * line_left_p.y);
	return c0 * p.x + c1 * p.y + c2;
}

void Rasterizer::draw_line(glm::vec2 p1, glm::vec2 p2, glm::vec4 color)
{
	glm::vec2 left = p1;
	glm::vec2 right = p2;
	if (p2.x < p1.x)
	{
		left = p2;
		right = p1;
	}

	float slope = (right.y - left.y) / ((float)right.x - (float)left.x);

	// vertical line
	if (left.x == right.x)
	{
		auto top = right;
		if (slope < 0)
			top = left;
		int n_pix = abs(left.y - right.y);
		while (n_pix--)
		{
			draw_point(top, color);
			top.y -= 1;
		}
	}
	// horizontal line
	else if (left.y == right.y)
	{

		int n_pix = right.x - left.x;
		while (n_pix--)
		{
			draw_point(left, color);
			left.x += 1;
		}
	}
	// diagonal 
	else
	{
		auto current = left;
		if (slope < 1.0 && slope > -1.0) // m(-1.0 , 1.0), more run than rise
		{
			int n_cols = right.x - left.x;
			while (n_cols--)
			{
				draw_point(current, color);
				current.x += 1; // move right
				if (slope < 0.0)
				{
					if (implicit_2d_line_eq(left, right, { current.x , current.y - 0.5 }) > 0.0)
						current.y -= 1.0;
				}
				else
				{
					if (implicit_2d_line_eq(left, right, { current.x , current.y + 0.5 }) < 0.0)
						current.y += 1.0;
				}
			}
		}
		else
		{
			int n_rows = abs(left.y - right.y);
			while (n_rows--)
			{
				draw_point(current, color);
				if (slope <= -1.0) // m (-inf , -1.0]
				{
					current.y -= 1; // move up
					if (implicit_2d_line_eq(left, right, { current.x + 0.5, current.y }) < 0.0)
						current.x += 1.0;
				}
				else // m (inf , 1.0]
				{
					current.y += 1;
					if (implicit_2d_line_eq(left, right, { current.x + 0.5, current.y }) > 0.0)
						current.x += 1.0;
				}
			}
		}
	}

}

void Rasterizer::draw_lines()
{
	auto& verticies = state->m_model.positions;

	for (auto& triangle : state->m_model.faces)
	{
		if (triangle.erase)
			continue;
		// face verts indices
		auto v0_index = triangle.p_indices.x;
		auto v1_index = triangle.p_indices.y;
		auto v2_index = triangle.p_indices.z;
		// face verts
		auto& v0 = verticies[v0_index];
		auto& v1 = verticies[v1_index];
		auto& v2 = verticies[v2_index];

		draw_line({ v0.x,v0.y }, { v1.x,v1.y }, glm::vec4{ 0xff,0x00,0x00,0xff });
		draw_line({ v0.x, v0.y }, { v2.x,v2.y }, glm::vec4{ 0xff,0x00,0x00,0xff });
		draw_line({ v1.x,v1.y }, { v2.x,v2.y }, glm::vec4{ 0xff,0x00,0x00,0xff });

	}
}