#pragma once

#include <glm/glm.hpp>
#include <vector>
#include <memory>
#include <mutex>
#include <atomic>

struct Face
{
	glm::vec3 p_indices;
	glm::vec3 n_indices;
	glm::vec3 t_indices;
};

struct Model
{
	std::vector<glm::vec4> positions;
	std::vector<glm::vec4> face_normals;
	std::vector<glm::vec4> colors;
	std::vector<glm::vec2> tex_coords;
	std::vector<Face> faces;
};

struct Camera
{
	glm::vec3 position;
	glm::vec3 lookat;
	glm::vec3 up;
};

struct SwapChain
{
	char* back_buffer;
	char* front_buffer;
	unsigned int frame_width;
	unsigned int frame_height;
	unsigned int frame_bytes_per_pixel;
	std::mutex m;
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
struct Window
{
	// window manager state
	int cursor_x;
	int cursor_y;
	unsigned  int win_width;
	unsigned int win_height;
	unsigned int win_bytes_per_pixel;
	bool win_resized;
	void* win_surface;
	std::mutex m;
};
struct Engine_State
{
	std::atomic_bool running;
	Window m_window;
	Model m_model_original;
	Model m_model;
	Camera m_camera;
	ViewVolume m_view_volume;
	DRAWING_MODE m_mode;
	SwapChain m_swapchain;
};