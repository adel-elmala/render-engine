﻿#include "../include/rasterizer.h"


Rasterizer::Rasterizer()
{
}

Rasterizer::~Rasterizer()
{
}

void Rasterizer::run()
{
	//ZoneScoped;

	switch (state->m_mode)
	{
	case DRAWING_MODE::POINTS:
		draw_points();
		break;
	case DRAWING_MODE::LINES:
		draw_lines();
		break;
	case DRAWING_MODE::TRIANGLES:
		draw_triangles();
		break;
	default:
		break;
	}
}

void Rasterizer::draw_points()
{
	//ZoneScoped;
	auto& faces = state->m_model.faces;
	size_t n_faces = faces.size();
	size_t thread_share = n_faces / state->n_threads;

	std::vector<std::thread> threads;
	auto thunk = [this, thread_share, &faces](size_t id)
		{
			size_t end = thread_share * (id + 1);
			for (size_t start = thread_share * id; start < end; ++start)
			{
				auto& face = faces[start];
				if (face.erase)
					continue;
				glm::vec3 p0 = state->m_model.positions[face.p_indices.x];
				glm::vec3 p1 = state->m_model.positions[face.p_indices.y];
				glm::vec3 p2 = state->m_model.positions[face.p_indices.z];

				auto c0 = state->m_model.colors[face.p_indices.x];
				draw_point(p0, c0);
				draw_point(p1, c0);
				draw_point(p2, c0);
			}
		};
	// launch threads
	for (size_t i = 0; i < state->n_threads; ++i)
		threads.emplace_back(thunk, i);

	// main thread handle the remaingings left
	for (size_t start = state->n_threads * thread_share; start < n_faces; ++start)
	{
		auto& face = faces[start];
		if (face.erase)
			continue;
		glm::vec3 p0 = state->m_model.positions[face.p_indices.x];
		glm::vec3 p1 = state->m_model.positions[face.p_indices.y];
		glm::vec3 p2 = state->m_model.positions[face.p_indices.z];

		auto c0 = state->m_model.colors[face.p_indices.x];
		draw_point(p0, c0);
		draw_point(p1, c0);
		draw_point(p2, c0);
	}

	// wait for the threads to finish
	for (size_t i = 0; i < state->n_threads; ++i)
		threads[i].join();
}

void Rasterizer::draw_point(glm::vec3& point, glm::u8vec4& color)
{
	//ZoneScoped;
	//std::scoped_lock lock(state->m_swapchain.m);

	float* z_buffer_current_value = (state->m_swapchain.z_buffer + (state->m_swapchain.frame_width * (int)point.y) + (int)point.x);
	if (point.z < *z_buffer_current_value)
		return;

	// update the z-buffer value
	*z_buffer_current_value = point.z;

	// pixel memory layout BB GG RR AA --- 0xAARRGGBB
	char* start =
		state->m_swapchain.back_buffer +
		((int)point.x * state->m_swapchain.frame_bytes_per_pixel) +
		((int)point.y * state->m_swapchain.frame_width * state->m_swapchain.frame_bytes_per_pixel);

	switch (state->m_swapchain.frame_bytes_per_pixel)
	{
	case 1:
		*start = color.b;
		break;
	case 2:
		*(uint16_t*)start = (color.b << 8) | color.g;
		break;
	case 3:
		*(uint16_t*)start = (color.b << 8) | color.g;
		start += 2;
		*start = color.r;
		break;
	case 4:
		*(uint32_t*)start = (((uint32_t)color.a) << 24) | (((uint32_t)color.r) << 16) | (((uint32_t)color.g) << 8) | ((uint32_t)color.b);
		break;
	}
}

void Rasterizer::draw_lines()
{
	//ZoneScoped;
	auto& verticies = state->m_model.positions;
	auto& colors = state->m_model.colors;

	auto& faces = state->m_model.faces;
	size_t n_faces = faces.size();
	size_t thread_share = n_faces / state->n_threads;

	std::vector<std::thread> threads;
	auto thunk = [this, thread_share, &colors, &verticies, &faces](size_t id)
		{
			size_t end = thread_share * (id + 1);
			for (size_t start = thread_share * id; start < end; ++start)
			{
				auto& triangle = faces[start];
				if (triangle.erase)
					continue;
				// face verts indices
				auto v0_index = triangle.p_indices.x;
				auto v1_index = triangle.p_indices.y;
				auto v2_index = triangle.p_indices.z;
				// face verts
				glm::vec3 v0 = verticies[v0_index];
				glm::vec3 v1 = verticies[v1_index];
				glm::vec3 v2 = verticies[v2_index];

				auto& c0 = colors[v0_index];

				draw_line(v0, v1, c0);
				draw_line(v0, v2, c0);
				draw_line(v1, v2, c0);
			}
		};
	// launch threads
	for (size_t i = 0; i < state->n_threads; ++i)
		threads.emplace_back(thunk, i);

	// main thread handle the remaingings left
	for (size_t start = state->n_threads * thread_share; start < n_faces; ++start)
	{
		auto& triangle = faces[start];
		if (triangle.erase)
			continue;
		// face verts indices
		auto v0_index = triangle.p_indices.x;
		auto v1_index = triangle.p_indices.y;
		auto v2_index = triangle.p_indices.z;
		// face verts
		glm::vec3 v0 = verticies[v0_index];
		glm::vec3 v1 = verticies[v1_index];
		glm::vec3 v2 = verticies[v2_index];

		auto& c0 = colors[v0_index];

		draw_line(v0, v1, c0);
		draw_line(v0, v2, c0);
		draw_line(v1, v2, c0);
	}

	// wait for the threads to finish
	for (size_t i = 0; i < state->n_threads; ++i)
		threads[i].join();
}

void Rasterizer::draw_line(glm::vec3& p1, glm::vec3& p2, glm::u8vec4& color)
{
	//ZoneScoped;

	glm::vec3 left = p1;
	glm::vec3 right = p2;
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

void Rasterizer::draw_triangles()
{
	//ZoneScoped;
	auto& faces = state->m_model.faces;
	size_t n_faces = faces.size();
	size_t thread_share = n_faces / state->n_threads;
	thread_share = (thread_share / 4) * 4;

	std::vector<std::thread> threads;
	auto thunk = [this, thread_share, &faces](size_t id)
		{
			size_t end = thread_share * (id + 1);
			for (size_t start = thread_share * id; start < end; start += 4)
			{
				draw_triangle(faces[start]);
				draw_triangle(faces[start + 1]);
				draw_triangle(faces[start + 2]);
				draw_triangle(faces[start + 3]);
			}
		};
	// launch threads
	for (size_t i = 0; i < state->n_threads; ++i)
		threads.emplace_back(thunk, i);

	// main thread handle the remaingings left
	for (size_t start = state->n_threads * thread_share; start < n_faces; ++start)
	{
		draw_triangle(faces[start]);
	}

	// wait for the threads to finish
	for (size_t i = 0; i < state->n_threads; ++i)
		threads[i].join();

}

void Rasterizer::draw_triangle(Face& triangle)
{
	//ZoneScoped;
	if (triangle.erase)
		return;
	auto& verticies = state->m_model.positions;
	auto& colors = state->m_model.colors;

	// face verts indices
	auto v0_index = triangle.p_indices.x;
	auto v1_index = triangle.p_indices.y;
	auto v2_index = triangle.p_indices.z;

	// face verts
	auto& v0 = verticies[v0_index];
	auto& v1 = verticies[v1_index];
	auto& v2 = verticies[v2_index];

	auto alpha_1 = implicit_2d_line_eq(v1, v2, v0);
	auto beta_1 = implicit_2d_line_eq(v0, v2, v1);

	auto off_screen_pixel = glm::ivec2{ 0,0 };
	auto off_screen_and_v0_on_same_side = (alpha_1 * implicit_2d_line_eq(v1, v2, off_screen_pixel)) > 0.0f;
	auto off_screen_and_v1_on_same_side = (beta_1 * implicit_2d_line_eq(v0, v2, off_screen_pixel)) > 0.0f;
	auto off_screen_and_v2_on_same_side = (implicit_2d_line_eq(v0, v1, v2) * implicit_2d_line_eq(v0, v1, off_screen_pixel)) > 0.0f;

	auto [bottom_left, top_right] = triangle_bounding_box(v0, v1, v2);

	for (unsigned int row = bottom_left.y; row <= top_right.y; ++row)
	{
		for (unsigned int col = bottom_left.x; col <= top_right.x; ++col)
		{
			// test if (row,col) is in the triangle
			auto candidate_pixel = glm::vec3{ col,row,0.0 };
			auto alpha = implicit_2d_line_eq(v1, v2, candidate_pixel) / alpha_1;
			auto beta = implicit_2d_line_eq(v0, v2, candidate_pixel) / beta_1;
			auto gamma = 1 - alpha - beta;

			if ((alpha >= 0 && beta >= 0 && gamma >= 0) and
				(alpha > 0 or off_screen_and_v0_on_same_side) and
				(beta > 0 or off_screen_and_v1_on_same_side) and
				(gamma > 0 or off_screen_and_v2_on_same_side))
			{
				candidate_pixel.z = v0.z * alpha + v1.z * beta + v2.z * gamma;
				//glm::u8vec4 c = glm::vec4{ colors[v0_index] } *alpha + glm::vec4{ colors[v1_index] } *beta + glm::vec4{ colors[v2_index] } *gamma

				// do a prespective-correct texture mapping
				float w_recp =
					(1 / state->m_model.verts_w_coords[v0_index]) * alpha +
					(1 / state->m_model.verts_w_coords[v1_index]) * beta +
					(1 / state->m_model.verts_w_coords[v2_index]) * gamma;

				glm::vec2 t =
					(state->m_model.tex_coords[triangle.t_indices.x] / state->m_model.verts_w_coords[v0_index]) * alpha +
					(state->m_model.tex_coords[triangle.t_indices.y] / state->m_model.verts_w_coords[v1_index]) * beta +
					(state->m_model.tex_coords[triangle.t_indices.z] / state->m_model.verts_w_coords[v2_index]) * gamma;
				t /= w_recp;

				auto c = sample_texture(state->m_model.textures[0], t);
				draw_point(candidate_pixel, c);
			}
		}
	}
}

std::pair<glm::ivec2, glm::ivec2> Rasterizer::triangle_bounding_box(const glm::ivec2& p0, const  glm::ivec2& p1, const  glm::ivec2& p2)
{
	//ZoneScoped;
	auto most_left = std::min(std::min(p0.x, p1.x), p2.x);
	auto most_right = std::max(std::max(p0.x, p1.x), p2.x);
	auto most_top = std::max(std::max(p0.y, p1.y), p2.y);
	auto most_bottom = std::min(std::min(p0.y, p1.y), p2.y);
	return { {most_left,most_bottom},{most_right,most_top} };
}

__forceinline float implicit_2d_line_eq(glm::vec2 line_left_p, glm::vec2 line_right_p, glm::vec2 p)
{
	//ZoneScoped;
	// p0 left  --  p1 right
	// f(x,y) = (y0 −y1)x +(x1 −x0)y +x0y1 −x1y0
	if (line_left_p.x > line_right_p.x)
	{
		std::swap(line_left_p, line_right_p);
	}
	auto c0 = line_left_p.y - line_right_p.y;
	auto c1 = line_right_p.x - line_left_p.x;
	auto c2 = (line_left_p.x * line_right_p.y) - (line_right_p.x * line_left_p.y);
	return c0 * p.x + c1 * p.y + c2;
}


glm::u8vec4 Rasterizer::sample_texture(Texture& t, glm::vec2 uv)
{
	size_t row = (t.height - 1) * uv.y;
	size_t col = (t.width - 1) * uv.x;

	char* start = t.data + row * (t.width * t.bytes_per_pixel) + (col * t.bytes_per_pixel);
	return glm::u8vec4{ *start , *(start + 1), *(start + 2) , 0xff };
}
