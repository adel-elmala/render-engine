#include "../include/common.h"
#include "../include/Application.h"

#define FAST_OBJ_IMPLEMENTATION
#include "../include/fast_obj.h"

#define STBI_FAILURE_USERMSG
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

Application::Application(const std::string& model_path)
{
	//ZoneScoped;
	m_model_path = model_path;
}

Application::~Application()
{
	//ZoneScoped;
	fast_obj_destroy((fastObjMesh*)m_mesh);
	for (auto& texture : state->m_model.textures)
		stbi_image_free(texture.data);

}
void Application::run()
{
	//ZoneScoped;
	parse_model(m_model_path);
	load_texture("../../assets/bunny/bunny-atlas.jpg");
	state->m_model_original = state->m_model;
}

void Application::parse_model(const std::string& path)
{
	//ZoneScoped;
	m_mesh = fast_obj_read(path.c_str());
	fastObjMesh* mesh = (fastObjMesh*)m_mesh;

	uint32_t p_count = mesh->position_count;
	uint32_t c_count = mesh->color_count;
	uint32_t t_count = mesh->texcoord_count;
	uint32_t n_count = mesh->normal_count;
	uint32_t f_count = mesh->face_count;

	state->m_model.positions.resize(p_count);
	state->m_model.verts_w_coords.resize(p_count);
	state->m_model.colors.resize(c_count);
	state->m_model.tex_coords.resize(t_count);
	state->m_model.face_normals.resize(n_count);
	state->m_model.faces.resize(f_count);

	// copy positions
	for (uint32_t i = 0, j = 0; j < p_count; i += 3, ++j)
	{
		state->m_model.positions[j] = glm::vec4{ mesh->positions[i],mesh->positions[i + 1],mesh->positions[i + 2] ,1.0f };
	}

	if (c_count >= f_count)
	{
		// copy colors
		for (uint32_t i = 0, j = 0; j < c_count; i += 3, ++j)
		{
			state->m_model.colors[j] = glm::vec4{ mesh->colors[i],mesh->colors[i + 1],mesh->colors[i + 2] ,1.0f };
		}
	}
	else
	{
		state->m_model.colors.resize(p_count);
		uint32_t counter = 0;
		// generate pseudo-colors
		for (uint32_t i = 0, j = 0; j < p_count; i += 3, ++j)
		{
			counter = std::rand();
			char red_channel = counter & 0x000000ff;
			char green_channel = (counter & 0x0000ff00) >> 8;
			char blue_channel = (counter & 0x00ff0000) >> 16;
			state->m_model.colors[j] = glm::u8vec4{ red_channel,green_channel,blue_channel ,0xff };
		}
	}

	// copy tex_coords	
	for (uint32_t i = 0, j = 0; j < t_count; i += 2, ++j)
	{
		state->m_model.tex_coords[j] = glm::vec2{ mesh->texcoords[i],mesh->texcoords[i + 1] };
	}

	// face normals
	for (uint32_t i = 0, j = 0; j < n_count; i += 3, ++j)
	{
		state->m_model.face_normals[j] = glm::vec4{ mesh->normals[i],mesh->normals[i + 1],mesh->normals[i + 2],0.0f };
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

	if (f_count < 200)
		state->n_threads = 2;
	else if (f_count < 2000)
		state->n_threads = 4;
	else if (f_count < 8000)
		state->n_threads = 16;
	else
		state->n_threads = std::thread::hardware_concurrency();
}

void Application::load_texture(const std::string& path)
{
	Texture t = {};
	t.bytes_per_pixel = state->m_swapchain.frame_bytes_per_pixel;

	stbi_set_flip_vertically_on_load(true);
	int n;
	t.data = (char*)stbi_load(path.c_str(), &(t.width), &(t.height), &n, t.bytes_per_pixel);

	if (!t.data)
		stbi_failure_reason();
	else
		state->m_model.textures.push_back(t);
}