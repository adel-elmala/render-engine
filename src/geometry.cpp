#include "../include/geometry.h"
#include <glm/gtc/matrix_transform.hpp> // translate, rotate, scale, perspective
#include <iostream>

Geometry::Geometry()
{
}

Geometry::~Geometry()
{
}

void Geometry::update_world_transform()
{
	// ZoneScoped;
	static char count = 0;
	static float dx = 0.0f;
	static float dy = 0.0f;
	if (state->backend == BACKEND_D3D11)
	{
		model_world_transform = glm::identity<glm::mat4>();
		model_world_transform = glm::translate(model_world_transform, glm::vec3{0, -400, (state->m_view_volume.near_plane + ((state->m_view_volume.far_plane - state->m_view_volume.near_plane) / 2))});
		model_world_transform = glm::scale(model_world_transform, glm::vec3{2.5f, 2.5f, 2.5f});
	}
	else
	{
		model_world_transform = glm::identity<glm::mat4>();
		model_world_transform = glm::translate(model_world_transform, glm::vec3{0, 0, state->m_view_volume.near_plane - (state->m_view_volume.near_plane - state->m_view_volume.far_plane) / 2});
		model_world_transform = glm::scale(model_world_transform, glm::vec3{0.15f, -0.15f, 0.15f});
	}

	if (state->m_window.enable_mouse_movement)
	{
		dx = (state->m_window.window_origin_x + state->m_window.cursor_x) / state->m_window.screen_width - 0.5f;
		dy = (state->m_window.window_origin_y + state->m_window.cursor_y) / state->m_window.screen_height - 0.5f;
	}
	model_world_transform = glm::rotate(model_world_transform, dx * glm::radians(360.0f), glm::vec3(0, 1, 0));
	model_world_transform = glm::rotate(model_world_transform, dy * glm::radians(360.0f), glm::vec3(1, 0, 0));
}

void Geometry::update_camera_transform()
{
	// ZoneScoped;

	// update the camera state according to Keyboard/Mouse input
	// if (state->m_window.enable_mouse_movement)
	// {
	// 	auto sensitivity = 0.3f;
	// 	state->m_window.mouse_yaw += state->m_window.cursor_dx * sensitivity;
	// 	state->m_window.mouse_pitch += state->m_window.cursor_dy * sensitivity;
	// 	// reset cursor deltas, otherwise the camera will continue to drift in the last registered direction
	// 	state->m_window.cursor_dx = 0;
	// 	state->m_window.cursor_dy = 0;

	// 	state->m_camera.lookat.x = cos(glm::radians(state->m_window.mouse_yaw)) * cos(glm::radians(state->m_window.mouse_pitch));
	// 	state->m_camera.lookat.y = sin(glm::radians(state->m_window.mouse_pitch));
	// 	state->m_camera.lookat.z = sin(glm::radians(state->m_window.mouse_yaw)) * cos(glm::radians(state->m_window.mouse_pitch));
	// }

	auto &eye = state->m_camera.position;
	auto gaze = state->m_camera.lookat;
	auto up = state->m_camera.up;

	// camera coords basis
	auto w = (glm::normalize(gaze));
	auto u = glm::normalize(glm::cross(w, up));
	auto v = glm::cross(u, w);
	if (state->backend == BACKEND_D3D11)
	{
		w = glm::normalize(gaze);
		u = -glm::normalize(glm::cross(w, up));
		v = -glm::cross(u, w);
	}

	if (state->m_window.move_cam_right)
		state->m_camera.position += u * state->m_camera.sensitivity;
	if (state->m_window.move_cam_left)
		state->m_camera.position -= u * state->m_camera.sensitivity;
	if (state->m_window.move_cam_forward)
		state->m_camera.position -= state->backend == BACKEND_D3D11 ? -w * state->m_camera.sensitivity : w * state->m_camera.sensitivity;
	if (state->m_window.move_cam_back)
		state->m_camera.position += state->backend == BACKEND_D3D11 ? -w * state->m_camera.sensitivity : w * state->m_camera.sensitivity;

	glm::mat4 translate_eye_to_origin(
		{1.0f, 0.0f, 0.0f, -eye.x},
		{0.0f, 1.0f, 0.0f, -eye.y},
		{0.0f, 0.0f, 1.0f, -eye.z},
		{0.0f, 0.0f, 0.0f, 1.0f});

	glm::mat4 align_basis(
		{u, 0.0f},
		{v, 0.0f},
		{w, 0.0f},
		{0.0f, 0.0f, 0.0f, 1.0f});

	world_camera_transform = glm::transpose(align_basis) * glm::transpose(translate_eye_to_origin);
}

void Geometry::update_perspective_transform()
{
	// ZoneScoped;

	auto n = state->m_view_volume.near_plane;
	auto f = state->m_view_volume.far_plane;
	auto t = state->m_view_volume.top_plane;
	auto b = state->m_view_volume.bottom_plane;
	auto l = state->m_view_volume.left_plane;
	auto r = state->m_view_volume.right_plane;

	glm::vec4 r0(2.0f / (r - l), 0.0f, 0.0f, -(r + l) / (r - l));
	glm::vec4 r1(0.0f, 2.0f / (t - b), 0.0, -(t + b) / (t - b));
	glm::vec4 r2(0.0f, 0.0f, 2.0f / (n - f), -(n + f) / (n - f));
	glm::vec4 r3(0.0f, 0.0f, 0.0f, 1.0f);

	if (state->backend == BACKEND_D3D11)
	{
		r2 = glm::vec4(0.0f, 0.0f, 1.0f / (f - n), (-n) / (f - n));
	}

	glm::mat4 orth(r0, r1, r2, r3);

	glm::vec4 rp0(n, 0.0f, 0.0f, 0.0f);
	glm::vec4 rp1(0.0f, n, 0.0f, 0.0f);
	glm::vec4 rp2(0.0f, 0.0f, n + f, -(f * n));
	glm::vec4 rp3(0.0f, 0.0f, 1.0f, 0.0f);

	glm::mat4 persp(rp0, rp1, rp2, rp3);

	camera_ndc_transform = glm::transpose(orth) * glm::transpose(persp);
}

void Geometry::update_viewport_transform()
{
	// ZoneScoped;

	auto nx = state->m_window.width;
	auto ny = state->m_window.height;
	auto nx_div_2 = nx / 2.0;
	auto ny_div_2 = ny / 2.0;

	glm::vec4 r1{nx_div_2, 0.0, 0.0, nx_div_2 - 0.5};
	glm::vec4 r2{0.0, ny_div_2, 0.0, ny_div_2 - 0.5};
	glm::vec4 r3{0.0, 0.0, 1.0, 0.0};
	glm::vec4 r4{0.0, 0.0, 0.0, 1.0};

	ndc_pixel_transform = glm::transpose(glm::mat4{r1, r2, r3, r4});
}
