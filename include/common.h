#pragma once

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#define GLM_FORCE_INLINE
#define GLM_FORCE_AVX2
#define GLM_FORCE_ALIGNED

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp> // translate, rotate, scale, perspective

#include <vector>
#include <memory>
#include <mutex>
#include <atomic>
#include <functional>

// #include <tracy/Tracy.hpp>
// #define TRACY_ENABLE
using Handle_t = void *;

enum DRAWING_MODE
{
	DRAWING_MODE_POINTS,
	DRAWING_MODE_LINES,
	DRAWING_MODE_TRIANGLES
};

struct Texture
{
	enum DIM
	{
		DIM_2D,
		DIM_CUBE
	};

	DIM dimensions;
	Handle_t texture_handle;
	Handle_t view_handle;
	size_t binding_point;
	const char *name;
	char *data[6];
	int width;
	int height;
	int bytes_per_pixel;
};

struct Render_Target
{
	Handle_t color_view_handle;
	Handle_t depth_view_handle;
	const char *name;
	Texture color;
	Texture depth;
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

// #pragma pack(push,1)
struct Vertex_attribute
{
	glm::vec4 pos;
	glm::vec4 normal;
	glm::vec2 uv;
};
// #pragma pack(pop)

struct Material
{
	glm::vec3 ka;
	glm::vec3 kd;
	glm::vec3 ks;
	uint32_t ns;
};



struct Camera
{
	glm::vec3 position;
	glm::vec3 lookat;
	glm::vec3 up;
	float sensitivity;
};

// TODO(adel): spot light
struct DirLight
{
	glm::vec3 direction;
	glm::vec3 color;
	float intensity;
};

struct PointLight
{
	glm::vec3 position;
	glm::vec3 color;
	float intensity;
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
	void *surface;
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
	BACKEND_D3D11,
	BACKEND_VULKAN
};



struct Uniform
{
	Handle_t handle;
	void *data;
	const char *name;
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

struct Bounding_Box
{
	float min_x, min_y, min_z;
	float max_x, max_y, max_z;
};

struct Model
{
	std::vector<Vertex_attribute> verts;
	std::vector<Texture> textures;
	Input_Layout layout;
	Material mtl;
	std::string map_kd;
	bool cast_shadow;
	glm::mat4 model_world_transfrom;
	Bounding_Box bb;
};

struct Scene
{
	std::vector<Model> models;
	std::vector<PointLight> pLights;
	std::vector<DirLight> dLights;
	Camera cam;
	Bounding_Box bb;
};

struct Engine_State
{
	Window window;
	Scene scene;
	ViewVolume view_volume;
	size_t n_threads;
	DRAWING_MODE mode;
	std::atomic_bool running;
	BACKEND backend;
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
	std::function<void(void)> update_uniforms; 
};

struct Program
{
	Shader vs;
	Shader ps;
	// Input_Layout attributes_layout;
	Handle_t vertex_buffer;
	Handle_t vertex_buffer_layout;
	size_t vertex_buffer_stride;
	size_t vertex_buffer_offset;
	size_t n_vert_attributes;
};

struct Render_Pass
{
	std::wstring name;
	Program used_prog;
	Render_Target render_target;
	DRAWING_MODE mode;
	glm::vec4 clear_color;
};
