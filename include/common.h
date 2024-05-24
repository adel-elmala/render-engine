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

struct Camera
{
	glm::vec3 position;
	glm::vec3 lookat;
	glm::vec3 up;
};

struct ViewVolume
{
	float near_plane;
	float far_plane;
	float left_plane;
	float right_plane;
	float top_plane;
	float bottom_plane;
};

enum DRAWING_MODE
{
	POINTS,
	LINES,
	TRIANGLES
};

struct Engine_State
{
	// window manager state
	unsigned int cursor_x;
	unsigned int cursor_y;
	unsigned int win_width;
	unsigned int win_height;
	bool running;

	// internal state
	Model m_model;
	Camera m_camera;
	ViewVolume m_view_volume;
	DRAWING_MODE m_mode;
};