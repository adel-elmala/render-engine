#include "../include/rasterizer.h"
#include <mutex>
Rasterizer::Rasterizer()
{
}

Rasterizer::~Rasterizer()
{
}


void Rasterizer::run()
{
	draw_points();
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
		*(uint32_t*)start = (uint32_t)start;//0xffffffff;
		break;
	}

}