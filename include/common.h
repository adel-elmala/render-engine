#pragma once

#include <glm/glm.hpp>
#include <vector>

struct Face
{
	glm::vec3 p_indices;
	glm::vec3 n_indices;
	glm::vec3 t_indices;
};

struct Model
{
	std::vector<glm::vec3> positions;
	std::vector<glm::vec3> colors;
	std::vector<glm::vec2> tex_coords;
	std::vector<glm::vec3> face_normals;
	std::vector<Face> faces;
};

struct Engine_State
{
	// window state
	unsigned int cursor_x;
	unsigned int cursor_y;
	bool running;

	Model m_model;
};
