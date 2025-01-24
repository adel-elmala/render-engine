#include <cstdlib>

#include "../include/renderEngine.h"
#include "../include/vulkanWrapper.h"
#include "../include/d3d11Wrapper.h"

int main(int argc, char** argv)
{

	// RenderEngine engine("../../assets/bunny/bunny.obj");
	// engine.start_engine();

	//VulkanWrapper app;
	D3D11Wrapper app;
	app.run();

	return EXIT_SUCCESS;
}