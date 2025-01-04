#include <cstdlib>

#include "../include/renderEngine.h"
#include "../include/vulkanWrapper.h"

int main(int argc, char** argv)
{

	RenderEngine engine("../../assets/bunny/bunny.obj");
	//RenderEngine engine("../../assets/cube3/cube.obj");

	// VulkanWrapper app;
	// app.run();

	return EXIT_SUCCESS;
}