#pragma once

#define GLM_FORCE_INLINE
#define GLM_FORCE_AVX2
#define GLM_FORCE_ALIGNED

#include <glm/glm.hpp>
#include <vector>
#include <memory>
#include <mutex>
#include <atomic>

//#include <tracy/Tracy.hpp>
//#define TRACY_ENABLE

struct Face
{
	glm::vec3 p_indices;
	glm::vec3 n_indices;
	glm::vec3 t_indices;
	bool erase;
};

struct Texture
{
	int width;
	int height;
	int bytes_per_pixel;
	char* data;
};

struct Model
{
	std::vector<glm::vec4> positions;
	std::vector<float> verts_w_coords;
	std::vector<glm::vec4> face_normals;
	std::vector<glm::u8vec4> colors;
	std::vector<glm::vec2> tex_coords;
	std::vector<Face> faces;
	std::vector<Texture> textures;
};

struct Camera
{
	glm::vec3 position;
	glm::vec3 lookat;
	glm::vec3 up;
	float sensitivity;
};

struct SwapChain
{
	std::mutex m;
	char* back_buffer;
	char* front_buffer;
	float* z_buffer;
	unsigned int frame_width;
	unsigned int frame_height;
	unsigned int frame_bytes_per_pixel;
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
	std::mutex m;
	float cursor_dx;
	float cursor_dy;
	float mouse_yaw;
	float mouse_pitch;
	bool enable_mouse_movement;
	bool move_cam_left;
	bool move_cam_right;
	bool move_cam_forward;
	bool move_cam_back;
	int win_width;
	int win_height;
	unsigned int win_bytes_per_pixel;
	bool win_resized;
	void* win_surface;
};

struct Engine_State
{
	Model m_model_original;
	Model m_model;
	Window m_window;
	SwapChain m_swapchain;
	Camera m_camera;
	DRAWING_MODE m_mode;
	ViewVolume m_view_volume;
	std::atomic_bool running;
	size_t n_threads;
};