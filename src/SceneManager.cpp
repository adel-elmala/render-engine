#include "../include/common.h"
#include "../include/SceneManager.h"
#include <glm/gtc/matrix_transform.hpp> // translate, rotate, scale, perspective

#define FAST_OBJ_IMPLEMENTATION
#include "../include/fast_obj.h"

#define STBI_FAILURE_USERMSG
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

SceneManager::SceneManager()
{
	// ZoneScoped;
}

SceneManager::~SceneManager()
{
	// ZoneScoped;
	for (auto &t : textures)
	{
		stbi_image_free(t.data[0]);
		if (t.dimensions = Texture::DIM_CUBE)
		{
			stbi_image_free(t.data[1]);
			stbi_image_free(t.data[2]);
			stbi_image_free(t.data[3]);
			stbi_image_free(t.data[4]);
			stbi_image_free(t.data[5]);
		}
	}
}

Model SceneManager::parse_model(const std::string &path)
{
	Model model;
	model.model_world_transfrom = glm::identity<glm::mat4>();
	auto mesh = (fastObjMesh *)fast_obj_read(path.c_str());

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

		vt_0.normal = glm::vec4{mesh->normals[vert_0_index.n * 3],
							 mesh->normals[vert_0_index.n * 3 + 1],
							 -mesh->normals[vert_0_index.n * 3 + 2],
							 0.0f};

		vt_1.normal = glm::vec4{mesh->normals[vert_1_index.n * 3],
							 mesh->normals[vert_1_index.n * 3 + 1],
							 -mesh->normals[vert_1_index.n * 3 + 2],
							 0.0f};

		vt_2.normal = glm::vec4{mesh->normals[vert_2_index.n * 3],
							 mesh->normals[vert_2_index.n * 3 + 1],
							 -mesh->normals[vert_2_index.n * 3 + 2],
							 0.0f};

		// glm::vec4 t01 = glm::normalize(vt_1.pos - vt_0.pos);
		// glm::vec4 t21 = glm::normalize(vt_2.pos - vt_0.pos);
		// vt_0.normal = glm::normalize(glm::vec4(glm::cross(glm::vec3(t21), glm::vec3(t01)), 0.0));
		// vt_1.normal = vt_0.normal;
		// vt_2.normal = vt_0.normal;

		model.verts.push_back(vt_0);
		model.verts.push_back(vt_1);
		model.verts.push_back(vt_2);
		i += 3;
	}
	// TODO(adel): fix this later, handle mtl array properly
	if (mesh->material_count != 0)
	{
		model.mtl.ka = glm::vec3(
			mesh->materials[0].Ka[0],
			mesh->materials[0].Ka[1],
			mesh->materials[0].Ka[2]);

		model.mtl.kd = glm::vec3(
			mesh->materials[0].Kd[0],
			mesh->materials[0].Kd[1],
			mesh->materials[0].Kd[2]);

		model.mtl.ks = glm::vec3(
			mesh->materials[0].Ks[0],
			mesh->materials[0].Ks[1],
			mesh->materials[0].Ks[2]);

		model.mtl.ns = mesh->materials[0].Ns;

		if (mesh->materials[0].map_Kd.path)
			model.map_kd = std::string(mesh->materials[0].map_Kd.path);
	}
	fast_obj_destroy(mesh);
	return model;
}

Texture SceneManager::load_texture(const std::string &path, bool flip_vertically)
{
	Texture t = {};
	t.dimensions = Texture::DIM_2D;

	t.bytes_per_pixel = 4;

	stbi_set_flip_vertically_on_load(flip_vertically);
	int n;
	t.data[0] = (char *)stbi_load(path.c_str(), &(t.width), &(t.height), &n, t.bytes_per_pixel);

	if (!t.data)
		stbi_failure_reason();

	textures.push_back(t);
	return t;
}

Env_map SceneManager::load_env_texture_cube(const std::string &path)
{
	Env_map env = {};
	env.front = load_texture(path + "front.jpg");
	env.back = load_texture(path + "back.jpg");
	env.left = load_texture(path + "left.jpg");
	env.right = load_texture(path + "right.jpg");
	env.top = load_texture(path + "top.jpg");
	env.bottom = load_texture(path + "bottom.jpg");

	return env;
}
