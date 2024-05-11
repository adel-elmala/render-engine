#pragma once

#include <string>
#include <vector>

//[TODO](adel) support scene description file input

struct Engine_State;

class Application
{
public:
	Application(const std::string& model_path);
	~Application();

	void run();

	void bind_state(Engine_State* engine_state) { if (engine_state) state = engine_state; }
private:

	void parse_model(const std::string& path);
	void* m_mesh; // single model mesh for now
	std::string m_model_path;
	Engine_State* state;
};