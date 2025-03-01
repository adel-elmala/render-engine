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
	std::vector<Texture> t;
	if(state->backend == BACKEND_SOFTWARE)
		t = state->m_model.m_cpu.textures;
	else
		t = state->m_model.m_gpu.textures;

	for (auto& texture : t)
		stbi_image_free(texture.data);
}

void Application::run()
{
	//ZoneScoped;
	if(state->backend == BACKEND_SOFTWARE)
	{
		parse_model_cpu(m_model_path);
		state->m_model_original.m_cpu = state->m_model.m_cpu;
	}
	else
	{
		parse_model_gpu(m_model_path);
		state->m_model_original.m_gpu = state->m_model.m_gpu;
	}
	load_texture(state->m_model.map_kd);
	// load_texture("../../assets/cube3/cube.png");
}

void Application::parse_model_gpu(const std::string& path)
{
	m_mesh = fast_obj_read(path.c_str());
	fastObjMesh *mesh = (fastObjMesh *)m_mesh;

	for (size_t i = 0; i < mesh->index_count;)
	{
		Vertex_attribute vt_0{};
		Vertex_attribute vt_1{};
		Vertex_attribute vt_2{};
		
		auto vert_0_index = mesh->indices[i];
		auto vert_1_index = mesh->indices[i + 1];
		auto vert_2_index = mesh->indices[i + 2];

		vt_0.pos = glm::vec4{mesh->positions[vert_0_index.p * 3],
							 mesh->positions[vert_0_index.p * 3 + 1],
							 -mesh->positions[vert_0_index.p * 3 + 2],
							 1.0f};

		vt_1.pos = glm::vec4{mesh->positions[vert_1_index.p * 3],
							 mesh->positions[vert_1_index.p * 3 + 1],
							 -mesh->positions[vert_1_index.p * 3 + 2],
							 1.0f};

		vt_2.pos = glm::vec4{mesh->positions[vert_2_index.p * 3],
							 mesh->positions[vert_2_index.p * 3 + 1],
							 -mesh->positions[vert_2_index.p * 3 + 2],
							 1.0f};

		vt_0.uv = glm::vec2{mesh->texcoords[vert_0_index.t * 2],
							mesh->texcoords[vert_0_index.t * 2 + 1]};

		vt_1.uv = glm::vec2{mesh->texcoords[vert_1_index.t * 2],
							mesh->texcoords[vert_1_index.t * 2 + 1]};

		vt_2.uv = glm::vec2{mesh->texcoords[vert_2_index.t * 2],
							mesh->texcoords[vert_2_index.t * 2 + 1]};

		glm::vec4 t01 = glm::normalize(vt_1.pos - vt_0.pos);
		glm::vec4 t21 = glm::normalize(vt_2.pos - vt_0.pos);
		vt_0.normal = glm::normalize(glm::vec4(glm::cross(glm::vec3(t21), glm::vec3(t01)), 0.0));
		// vt_0.normal = glm::normalize(glm::vec4(glm::cross(glm::vec3(t01), glm::vec3(t21)), 0.0));
		vt_1.normal = vt_0.normal;
		vt_2.normal = vt_0.normal;

		state->m_model.m_gpu.verts.push_back(vt_0);
		state->m_model.m_gpu.verts.push_back(vt_1);
		state->m_model.m_gpu.verts.push_back(vt_2);
		i += 3;
	}
	// TODO(adel): fix this later, handle mtl array properly
	if (mesh->material_count != 0)
	{
		state->m_model.mtl.ka = glm::vec3(
			mesh->materials[0].Ka[0],
			mesh->materials[0].Ka[1],
			mesh->materials[0].Ka[2]);

		state->m_model.mtl.kd = glm::vec3(
			mesh->materials[0].Kd[0],
			mesh->materials[0].Kd[1],
			mesh->materials[0].Kd[2]);

		state->m_model.mtl.ks = glm::vec3(
			mesh->materials[0].Ks[0],
			mesh->materials[0].Ks[1],
			mesh->materials[0].Ks[2]);

		state->m_model.mtl.ns = mesh->materials[0].Ns;

		if (mesh->materials[0].map_Kd.path)
			state->m_model.map_kd = std::string(mesh->materials[0].map_Kd.path);
	}
}

void Application::parse_model_cpu(const std::string& path)
{
	//ZoneScoped;
	m_mesh = fast_obj_read(path.c_str());
	fastObjMesh* mesh = (fastObjMesh*)m_mesh;

	int32_t p_count = mesh->position_count - 1;
	int32_t c_count = mesh->color_count;
	int32_t t_count = mesh->texcoord_count - 1;
	int32_t n_count = mesh->normal_count - 1;
	int32_t f_count = mesh->face_count;

	state->m_model.m_cpu.positions.resize(p_count);
	state->m_model.m_cpu.verts_w_coords.resize(p_count);
	state->m_model.m_cpu.colors.resize(c_count);
	state->m_model.m_cpu.tex_coords.resize(t_count);
	state->m_model.m_cpu.face_normals.resize(n_count);
	state->m_model.m_cpu.faces.resize(f_count);

	// copy positions
	for (uint32_t i = 3, j = 0; j < p_count; i += 3, ++j)
	{
		state->m_model.m_cpu.positions[j] = glm::vec4{ mesh->positions[i],mesh->positions[i + 1],mesh->positions[i + 2] ,1.0f };
	}

	if (c_count >= f_count)
	{
		// copy colors
		for (uint32_t i = 0, j = 0; j < c_count; i += 3, ++j)
		{
			state->m_model.m_cpu.colors[j] = glm::vec4{ mesh->colors[i],mesh->colors[i + 1],mesh->colors[i + 2] ,1.0f };
		}
	}
	else
	{
		state->m_model.m_cpu.colors.resize(p_count);
		uint32_t counter = 0;
		// generate pseudo-colors
		for (uint32_t i = 0, j = 0; j < p_count; i += 3, ++j)
		{
			counter = std::rand();
			char red_channel = counter & 0x000000ff;
			char green_channel = (counter & 0x0000ff00) >> 8;
			char blue_channel = (counter & 0x00ff0000) >> 16;
			state->m_model.m_cpu.colors[j] = glm::u8vec4{ red_channel,green_channel,blue_channel ,0xff };
		}
	}

	// copy tex_coords	
	for (uint32_t i = 2, j = 0; j < t_count; i += 2, ++j)
	{
		state->m_model.m_cpu.tex_coords[j] = glm::vec2{ mesh->texcoords[i],mesh->texcoords[i + 1] };
	}

	// face normals
	for (uint32_t i = 3, j = 0; j < n_count; i += 3, ++j)
	{
		state->m_model.m_cpu.face_normals[j] = glm::vec4{ mesh->normals[i],mesh->normals[i + 1],mesh->normals[i + 2],0.0f };
	}

	// copy faces
	for (uint32_t i = 0, j = 0; j < f_count; i += 3, ++j)
	{
		Face tmp{};
		tmp.p_indices = glm::vec3{ mesh->indices[i].p - 1 ,mesh->indices[i + 1].p - 1,mesh->indices[i + 2].p - 1 };
		tmp.n_indices = glm::vec3{ mesh->indices[i].n - 1,mesh->indices[i + 1].n - 1,mesh->indices[i + 2].n - 1 };
		tmp.t_indices = glm::vec3{ mesh->indices[i].t - 1,mesh->indices[i + 1].t - 1 ,mesh->indices[i + 2].t - 1 };

		// generate face normals if not found in the model
		if (n_count < 1)
		{
			auto v0 = state->m_model.m_cpu.positions[tmp.p_indices[0]];
			auto v1 = state->m_model.m_cpu.positions[tmp.p_indices[1]];
			auto v2 = state->m_model.m_cpu.positions[tmp.p_indices[2]];

			auto e0 = v0 - v1;
			auto e1 = v2 - v1;
			auto n = glm::normalize(
				glm::cross(
					glm::vec3(e0.x, e0.y, e0.z),
					glm::vec3(e1.x, e1.y, e1.z))
			);
			state->m_model.m_cpu.face_normals.push_back(glm::vec4(n, 0.0f));
			tmp.n_indices = glm::vec3{ j,j,j };
		}

		state->m_model.m_cpu.faces[j] = tmp;
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

	t.bytes_per_pixel = state->backend == BACKEND_SOFTWARE ? state->m_swapchain.frame_bytes_per_pixel : 4;

	stbi_set_flip_vertically_on_load(true);
	int n;
	t.data = (char*)stbi_load(path.c_str(), &(t.width), &(t.height), &n, t.bytes_per_pixel);

	if (!t.data)
		stbi_failure_reason();
	else
	{
		if (state->backend == BACKEND_SOFTWARE)
			state->m_model.m_cpu.textures.push_back(t);
		else
			state->m_model.m_gpu.textures.push_back(t);
	}
}