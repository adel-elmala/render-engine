#pragma once

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

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
using Handle_t = void *;

enum DRAWING_MODE
{
	DRAWING_MODE_POINTS,
	DRAWING_MODE_LINES,
	DRAWING_MODE_TRIANGLES
};

struct Face
{
	glm::vec3 p_indices;
	glm::vec3 n_indices;
	glm::vec3 t_indices;
	bool erase;
};

struct Texture
{
	Handle_t texture_handle;
	Handle_t view_handle;
	size_t binding_point;
	const char* name;
	char* data;
	int width;
	int height;
	int bytes_per_pixel;
};

struct Env_map
{
	Texture front;
	Texture back;
	Texture left;
	Texture right;
	Texture top;
	Texture bottom;
};

struct Model_CPU
{
	std::vector<glm::vec4> positions;
	std::vector<float> verts_w_coords;
	std::vector<glm::vec4> face_normals;
	std::vector<glm::u8vec4> colors;
	std::vector<glm::vec2> tex_coords;
	std::vector<Face> faces;
	std::vector<Texture> textures;
};

// #pragma pack(push,1)
struct Vertex_attribute
{
	glm::vec4 pos;
	glm::vec4 normal;
	glm::vec2 uv;
};
// #pragma pack(pop)

struct Model_GPU
{
	std::vector<Vertex_attribute> verts;
	std::vector<Texture> textures;
};

struct Material
{
	glm::vec3 ka;
	glm::vec3 kd;
	glm::vec3 ks;
	uint32_t ns; 
};

struct Model
{
	Model_CPU m_cpu;
	Model_GPU m_gpu;
	Env_map env_map;
	Material mtl;
	std::string map_kd;
};

struct Camera
{
	glm::vec3 position;
	glm::vec3 lookat;
	glm::vec3 up;
	float sensitivity;
};

struct DirLight
{
	glm::vec3 direction;
	glm::vec3 color;
	float intensity;
};

struct PointLight
{
	glm::vec3 position;
	// float padding_0 = 0.0f;
	glm::vec3 color;
	// float padding_1 = 0.0f;
	float intensity;
	// glm::vec3 padding_2{};
};
// TODO(adel): spot light

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

struct Window
{
	std::mutex m;
	void* surface;
	HWND win32_win;
	int width;
	int height;
	int screen_width;
	int screen_height;
	int window_origin_x;
	int window_origin_y;
	unsigned int bytes_per_pixel;
	float cursor_dx;
	float cursor_dy;
	float cursor_x;
	float cursor_y;
	float mouse_yaw;
	float mouse_pitch;
	bool enable_mouse_movement;
	bool move_cam_left;
	bool move_cam_right;
	bool move_cam_forward;
	bool move_cam_back;
	bool resized;
};

enum BACKEND
{
	BACKEND_SOFTWARE,
	BACKEND_D3D11,
	BACKEND_VULKAN
};

struct Engine_State
{
	Model m_model_original;
	Model m_model;
	Window m_window;
	SwapChain m_swapchain;
	Camera m_camera;
	ViewVolume m_view_volume;
	size_t n_threads;
	DRAWING_MODE m_mode;
	std::atomic_bool running;
	BACKEND backend;
};

struct Uniform
{
	Handle_t handle;
	void* data;
	const char* name;
	size_t size;
	size_t binding_point;
};

enum FORMAT
{
	FORMAT_R32G32B32A32_FLOAT,
	FORMAT_R32G32_FLOAT,
};

enum V_ATTRIBUTE_FREQ
{
	V_ATTRIBUTE_FREQ_PER_VERTEX,
	V_ATTRIBUTE_FREQ_PER_INSTANCE,
};

enum V_ATTRIBUTE_TYPE
{
	V_ATTRIBUTE_TYPE_POSITION,
	V_ATTRIBUTE_TYPE_NORMAL,
	V_ATTRIBUTE_TYPE_TEXTURE_COORD,
};

struct Element_Desc
{
	V_ATTRIBUTE_TYPE type;
	FORMAT format;
	V_ATTRIBUTE_FREQ freq;
};

struct Input_Layout
{
	std::vector<Element_Desc> elements;
};

enum SHADER_STAGE
{
	SHADER_STAGE_VERTEX,
	SHADER_STAGE_PIXEL,
};

struct Shader
{
	Handle_t handle;
	std::wstring path;
	std::string entry;
	std::vector<Uniform> uniforms;
	std::vector<Texture> textures;
};

struct Program
{
	Shader vs;
	Shader ps;
	// Input_Layout attributes_layout;
	Handle_t vertex_buffer;
	Handle_t vertex_buffer_layout;
};

struct Render_Pass
{
	Program used_prog;
	Texture render_target;
};
