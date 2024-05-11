#include "../include/common.h"
#include "../include/Application.h"

#define FAST_OBJ_IMPLEMENTATION
#include "../include/fast_obj.h"

Application::Application(const std::string& model_path)
{
	m_model_path = model_path;
}

Application::~Application()
{
	fast_obj_destroy((fastObjMesh*)m_mesh);
}
void Application::run()
{
	parse_model(m_model_path);
}

void Application::parse_model(const std::string& path)
{
	m_mesh = fast_obj_read(path.c_str());
	fastObjMesh* mesh = (fastObjMesh*)m_mesh;

	uint32_t p_count = mesh->position_count - 1;
	uint32_t c_count = mesh->color_count;
	uint32_t t_count = mesh->texcoord_count - 1;
	uint32_t n_count = mesh->normal_count - 1;
	uint32_t f_count = mesh->face_count;

	
	state->m_model.positions.resize(p_count);
	state->m_model.colors.resize(c_count);
	state->m_model.tex_coords.resize(t_count);
	state->m_model.face_normals.resize(n_count);
	state->m_model.faces.resize(f_count);

	// copy positions
	for (uint32_t i = 3, j = 0; j < p_count; i += 3, ++j)
	{
		state->m_model.positions[j] = glm::vec3{ mesh->positions[i],mesh->positions[i + 1],mesh->positions[i + 2] };
	}

	// copy colors
	for (uint32_t i = 0, j = 0; j < c_count; i += 3, ++j)
	{
		state->m_model.colors[j] = glm::vec3{ mesh->colors[i],mesh->colors[i + 1],mesh->colors[i + 2] };
	}

	// copy tex_coords	
	for (uint32_t i = 2, j = 0; j < t_count; i += 2, ++j)
	{
		state->m_model.tex_coords[j] = glm::vec2{ mesh->texcoords[i],mesh->texcoords[i + 1] };
	}

	// face normals
	for (uint32_t i = 3, j = 0; j < n_count; i += 3, ++j)
	{
		state->m_model.face_normals[j] = glm::vec3{ mesh->normals[i],mesh->normals[i + 1],mesh->normals[i + 2] };
	}

	// copy faces
	for (uint32_t i = 0, j = 0; j < f_count; i += 3, ++j)
	{
		Face tmp{};
		tmp.p_indices = glm::vec3{ mesh->indices[i].p ,mesh->indices[i + 1].p ,mesh->indices[i + 2].p };
		tmp.n_indices = glm::vec3{ mesh->indices[i].n ,mesh->indices[i + 1].n ,mesh->indices[i + 2].n };
		tmp.t_indices = glm::vec3{ mesh->indices[i].t ,mesh->indices[i + 1].t ,mesh->indices[i + 2].t };

		state->m_model.faces[j] = tmp;
	}
}

