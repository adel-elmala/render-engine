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
	static float dx = 0.0f;
	static float dy = 0.0f;
	static float old_cursor_x = 0.0f;
	static float old_cursor_y = 0.0f;
	if (state->backend == BACKEND_D3D11)
	{
		model_world_transform = glm::identity<glm::mat4>();
		model_world_transform = glm::translate(model_world_transform, glm::vec3{0, -10, (state->view_volume.near_plane + ((state->view_volume.far_plane - state->view_volume.near_plane) / 2))});
		model_world_transform = glm::scale(model_world_transform, glm::vec3{.05f, .05f, .05f});
	}
	else
	{
		model_world_transform = glm::identity<glm::mat4>();
		model_world_transform = glm::translate(model_world_transform, glm::vec3{0, 0, state->view_volume.near_plane - (state->view_volume.near_plane - state->view_volume.far_plane) / 2});
		model_world_transform = glm::scale(model_world_transform, glm::vec3{0.15f, -0.15f, 0.15f});
	}

	if (state->window.enable_mouse_movement)
	{
		if (old_cursor_x != state->window.cursor_x)
		{
			old_cursor_x = state->window.cursor_x;
			dx += state->window.cursor_dx;
		}
		if (old_cursor_y != state->window.cursor_y)
		{
			old_cursor_y = state->window.cursor_y;
			dy += state->window.cursor_dy;
		}
	}
	model_world_transform = glm::rotate(model_world_transform, dx * glm::radians(1.0f), glm::vec3(0, 1, 0));
	model_world_transform = glm::rotate(model_world_transform, dy * glm::radians(1.0f), glm::vec3(1, 0, 0));

	model_world_transform_mouse = glm::rotate(glm::identity<glm::mat4>(), dx * glm::radians(1.0f), glm::vec3(0, 1, 0));
	// model_world_transform_mouse = glm::rotate(model_world_transform_mouse, dy * glm::radians(1.0f), glm::vec3(1, 0, 0));
}

void Geometry::update_camera_transform()
{
	// ZoneScoped;
	auto &eye = state->scene.cam.position;
	auto gaze = state->scene.cam.lookat;
	auto up = state->scene.cam.up;

	// camera coords basis
	auto w = glm::normalize(gaze);
	auto u = glm::normalize(glm::cross(w, up));
	auto v = glm::cross(u, w);
	if (state->backend == BACKEND_D3D11)
	{
		w = glm::normalize(gaze);
		u = -glm::normalize(glm::cross(w, up));
		v = -glm::cross(u, w);
	}

	if (state->window.move_cam_right)
		eye += u * state->scene.cam.sensitivity;
	if (state->window.move_cam_left)
		eye -= u * state->scene.cam.sensitivity;
	if (state->window.move_cam_forward)
		eye -= state->backend == BACKEND_D3D11 ? -w * state->scene.cam.sensitivity : w * state->scene.cam.sensitivity;
	if (state->window.move_cam_back)
		eye += state->backend == BACKEND_D3D11 ? -w * state->scene.cam.sensitivity : w * state->scene.cam.sensitivity;

	gaze = eye + gaze;
	world_camera_transform = glm::lookAt(eye, gaze, up);
}

void Geometry::update_perspective_transform()
{
	// ZoneScoped;
	auto n = state->view_volume.near_plane;
	auto f = state->view_volume.far_plane;

	camera_ndc_transform = glm::perspective(state->view_volume.fovy, (float)state->window.width / state->window.height, n, f);
}
