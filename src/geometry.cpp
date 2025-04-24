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
		model_world_transform = glm::translate(model_world_transform, glm::vec3{0, -400, (state->view_volume.near_plane + ((state->view_volume.far_plane - state->view_volume.near_plane) / 2))});
		model_world_transform = glm::scale(model_world_transform, glm::vec3{2.5f, 2.5f, 2.5f});
	}
	else
	{
		model_world_transform = glm::identity<glm::mat4>();
		model_world_transform = glm::translate(model_world_transform, glm::vec3{0, 0, state->view_volume.near_plane - (state->view_volume.near_plane - state->view_volume.far_plane) / 2});
		model_world_transform = glm::scale(model_world_transform, glm::vec3{0.15f, -0.15f, 0.15f});
	}

	if (state->window.enable_mouse_movement)
	{
		dx = (state->window.window_origin_x + state->window.cursor_x) / state->window.screen_width - 0.5f;
		dy = (state->window.window_origin_y + state->window.cursor_y) / state->window.screen_height - 0.5f;
	}
	model_world_transform = glm::rotate(model_world_transform, dx * glm::radians(360.0f), glm::vec3(0, 1, 0));
	model_world_transform = glm::rotate(model_world_transform, dy * glm::radians(360.0f), glm::vec3(1, 0, 0));
}

void Geometry::update_camera_transform()
{
	// ZoneScoped;

	// update the camera state according to Keyboard/Mouse input
	// if (state->window.enable_mouse_movement)
	// {
	// 	auto sensitivity = 0.3f;
	// 	state->window.mouse_yaw += state->window.cursor_dx * sensitivity;
	// 	state->window.mouse_pitch += state->window.cursor_dy * sensitivity;
	// 	// reset cursor deltas, otherwise the camera will continue to drift in the last registered direction
	// 	state->window.cursor_dx = 0;
	// 	state->window.cursor_dy = 0;

	// 	state->scene.cam.lookat.x = cos(glm::radians(state->window.mouse_yaw)) * cos(glm::radians(state->window.mouse_pitch));
	// 	state->scene.cam.lookat.y = sin(glm::radians(state->window.mouse_pitch));
	// 	state->scene.cam.lookat.z = sin(glm::radians(state->window.mouse_yaw)) * cos(glm::radians(state->window.mouse_pitch));
	// }

	auto &eye = state->scene.cam.position;
	auto gaze = state->scene.cam.lookat;
	auto up = state->scene.cam.up;

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

	if (state->window.move_cam_right)
		state->scene.cam.position += u * state->scene.cam.sensitivity;
	if (state->window.move_cam_left)
		state->scene.cam.position -= u * state->scene.cam.sensitivity;
	if (state->window.move_cam_forward)
		state->scene.cam.position -= state->backend == BACKEND_D3D11 ? -w * state->scene.cam.sensitivity : w * state->scene.cam.sensitivity;
	if (state->window.move_cam_back)
		state->scene.cam.position += state->backend == BACKEND_D3D11 ? -w * state->scene.cam.sensitivity : w * state->scene.cam.sensitivity;

	gaze = eye + gaze;
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

	// world_camera_transform = glm::transpose(align_basis) * glm::transpose(translate_eye_to_origin);
	world_camera_transform = glm::lookAtLH(eye, gaze, up);
}

void Geometry::update_perspective_transform()
{
	// ZoneScoped;

	auto n = state->view_volume.near_plane;
	auto f = state->view_volume.far_plane;
	auto t = state->view_volume.top_plane;
	auto b = state->view_volume.bottom_plane;
	auto l = state->view_volume.left_plane;
	auto r = state->view_volume.right_plane;

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

	// camera_ndc_transform = glm::transpose(orth) * glm::transpose(persp);
	// new_mats_light.world_camera = glm::lookAtLH(used_plight.position, light_dir, glm::vec3(0, 1, 0));
	float fovy = atan2f(t, n) * 2;
	camera_ndc_transform = glm::perspectiveLH(fovy, (float)state->window.width / state->window.height, n, f);

}

void Geometry::update_viewport_transform()
{
	// ZoneScoped;

	auto nx = state->window.width;
	auto ny = state->window.height;
	auto nx_div_2 = nx / 2.0;
	auto ny_div_2 = ny / 2.0;

	glm::vec4 r1{nx_div_2, 0.0, 0.0, nx_div_2 - 0.5};
	glm::vec4 r2{0.0, ny_div_2, 0.0, ny_div_2 - 0.5};
	glm::vec4 r3{0.0, 0.0, 1.0, 0.0};
	glm::vec4 r4{0.0, 0.0, 0.0, 1.0};

	ndc_pixel_transform = glm::transpose(glm::mat4{r1, r2, r3, r4});
}
