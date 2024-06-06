#include "../include/geometry.h"

#include <glm/gtc/matrix_transform.hpp> // translate, rotate, scale, perspective

Geometry::Geometry()
{
}

Geometry::~Geometry()
{
}

void Geometry::update_world_transform()
{
	static float count = 0;
	model_world_transform = glm::identity<glm::mat4>();
	model_world_transform = glm::translate(model_world_transform, glm::vec3{ 0,0 ,-150.0f});
	model_world_transform = glm::scale(model_world_transform, glm::vec3{ 0.25f,-0.25f ,0.25f });
	model_world_transform = glm::rotate(model_world_transform, glm::radians((float)(count += 1)), glm::vec3{ 0.0f,1.0f ,0.0f });
	//model_world_transform = glm::translate(model_world_transform, glm::vec3{ 0.0f,0.0f ,-120.0f });
	//model_world_transform = glm::scale(model_world_transform, glm::vec3{ 40.0f,-40.0f ,40.0f });
	//model_world_transform = glm::rotate(model_world_transform, glm::radians((float)(count += 0.25)), glm::vec3{ 0.0f,1.0f ,0.0f });
}

void Geometry::update_camera_transform()
{
	auto eye = state->m_camera.position;
	auto gaze = state->m_camera.lookat - state->m_camera.position;
	auto up = state->m_camera.up;
	// camera coords basis
	auto w = -(glm::normalize(gaze));
	auto u = glm::normalize(glm::cross(up, w));
	auto v = glm::cross(w, u);

	glm::mat4 translate_eye_to_origin(
		{ 1.0f, 0.0f, 0.0f, -eye.x },
		{ 0.0f, 1.0f, 0.0f, -eye.y },
		{ 0.0f, 0.0f, 1.0f, -eye.z },
		{ 0.0f, 0.0f, 0.0f, 1.0f });

	glm::mat4 align_basis(
		{ u,0.0f },
		{ v,0.0f },
		{ w,0.0f },
		{ 0.0f, 0.0f, 0.0f, 1.0f });

	world_camera_transform = glm::transpose(align_basis) * glm::transpose(translate_eye_to_origin);

}

void Geometry::update_perspective_transform()
{
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
	auto nx = state->m_swapchain.frame_width;
	auto ny = state->m_swapchain.frame_height;
	auto nx_div_2 = nx / 2.0;
	auto ny_div_2 = ny / 2.0;

	glm::vec4 r1{ nx_div_2,0.0,0.0,nx_div_2 - 0.5 };
	glm::vec4 r2{ 0.0,ny_div_2,0.0,ny_div_2 - 0.5 };
	glm::vec4 r3{ 0.0,0.0,1.0,0.0 };
	glm::vec4 r4{ 0.0,0.0,0.0,1.0 };

	ndc_pixel_transform = glm::transpose(glm::mat4{ r1,r2,r3,r4 });
}

void Geometry::send_to_camera_space()
{
	update_world_transform();
	update_camera_transform();
	auto m = world_camera_transform * model_world_transform;

	for (auto& vertex_pos : state->m_model.positions)
	{
		vertex_pos = m * vertex_pos;
	}

	auto mn = glm::transpose(glm::inverse(m));
	for (auto& face_normal : state->m_model.face_normals)
	{
		face_normal = mn * face_normal;
	}
}

void Geometry::send_to_ndc_space()
{
	update_perspective_transform();

	auto m = camera_ndc_transform;

	for (auto& vertex_pos : state->m_model.positions)
	{
		auto v = m * vertex_pos;
		vertex_pos = v / v.w;
	}

	auto mn = glm::transpose(glm::inverse(m));
	// NOTE(adel): do we need the normals ? 
	for (auto& face_normal : state->m_model.face_normals)
	{
		auto r = mn * face_normal;
		face_normal = r / r.w;
	}
}

void Geometry::send_to_pixel_space()
{
	update_viewport_transform();

	auto m = ndc_pixel_transform;

	for (auto& vertex_pos : state->m_model.positions)
	{
		vertex_pos = m * vertex_pos;
	}

	auto mn = glm::transpose(glm::inverse(m));
	// NOTE(adel): do we need the normals ? 
	for (auto& face_normal : state->m_model.face_normals)
	{
		face_normal = mn * face_normal;
	}
}

void Geometry::lighting_calc()
{
	send_to_camera_space();
	// do lighting stuff here
}

void Geometry::clipping()
{
	send_to_ndc_space();
	// clipping base on drawing mode (points / lines / triangles)
	switch (state->m_mode)
	{
	case DRAWING_MODE::POINTS:
		// TODO(adel)
		break;
	case DRAWING_MODE::LINES:
		// TODO(adel)
		clip_triangles();
		break;
	case DRAWING_MODE::TRIANGLES:
		clip_triangles();
		break;
	default:
		break;
	}
}

void Geometry::run()
{
	// run lighting stage
	lighting_calc();
	// run clipping stage
	clipping();
	// send the remaining vertecies to pixel space
	send_to_pixel_space();
}

__forceinline bool Geometry::in_view_volume(glm::vec4 point)
{
	bool bx = (point.x <= 1.0f) && (point.x >= -1.0f);
	bool by = (point.y <= 1.0f) && (point.y >= -1.0f);
	bool bz = (point.z <= 1.0f) && (point.z >= -1.0f);
	return bx && by && bz;
}
void Geometry::clip_triangles()
{
	auto& verticies = state->m_model.positions;

	for (auto& triangle : state->m_model.faces)
	{
		// face verts indices
		auto v0_index = triangle.p_indices[0];
		auto v1_index = triangle.p_indices[1];
		auto v2_index = triangle.p_indices[2];
		// face verts
		auto& v0 = verticies[v0_index];
		auto& v1 = verticies[v1_index];
		auto& v2 = verticies[v2_index];

		auto v0_in = in_view_volume(v0);
		auto v1_in = in_view_volume(v1);
		auto v2_in = in_view_volume(v2);

		auto n_vert_out = 3 - (v0_in + v1_in + v2_in);

		switch (n_vert_out)
		{
			// TODO(adel): handle partially clipped triangles
		case 3:
		case 2:
		case 1:
			triangle.erase = true;
			break;
		case 0:
			triangle.erase = false;
			break;
		default:
			break;
		}


	}
}


glm::vec4 Geometry::transform_model_to_window(glm::vec3 v_model_space)
{
	update_world_transform();
	update_camera_transform();
	update_perspective_transform();
	update_viewport_transform();

	return ndc_pixel_transform * camera_ndc_transform * world_camera_transform * model_world_transform * glm::vec4{ v_model_space,1.0 };;
}

