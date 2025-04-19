#pragma once

#include <string>
#include <vector>

//[TODO](adel) support scene description file input

class SceneManager
{
public:
	SceneManager();
	~SceneManager();

	Model parse_model(const std::string &path);
	Texture load_texture(const std::string &path, bool flip_vertically = false);
	Env_map load_env_texture_cube(const std::string &path);
private:
	std::vector<Texture> textures;
};
