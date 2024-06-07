cmake_minimum_required(VERSION 3.16)

project(renderEngine)

set(CMAKE_CXX_STANDARD 20)

# Engine src
set(SRC "src/main.cpp" "src/renderEngine.cpp" "src/windowManager.cpp" "src/Application.cpp" "src/geometry.cpp" "src/rasterizer.cpp")
set(HEADER "include/renderEngine.h" "include/windowManager.h" "include/geometry.h" "include/rasterizer.h")

# vulkan module src
set(VULKAN_SRC "src/vulkanWrapper.cpp" "include/vulkanWrapper.h")

# logger module src
set(LOGGER_SRC "src/logger.cpp" "include/logger.h")

# required external libs
find_package(Vulkan REQUIRED)
find_package(glfw3 CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(SDL2 CONFIG REQUIRED)
find_package(Tracy CONFIG REQUIRED)
# logger module library 
add_library(logger_module STATIC ${LOGGER_SRC})
target_link_libraries(logger_module PUBLIC)


# vulkan module library 
add_library(Vulkan_module STATIC ${VULKAN_SRC})
target_link_libraries(Vulkan_module PUBLIC glfw glm::glm Vulkan::Vulkan logger_module)

# render engine exe.
add_executable(${PROJECT_NAME} ${SRC})
add_definitions(/arch:AVX2)

target_link_libraries(${PROJECT_NAME}
$<TARGET_NAME_IF_EXISTS:SDL2::SDL2main> # SDL2
$<IF:$<TARGET_EXISTS:SDL2::SDL2>,SDL2::SDL2,SDL2::SDL2-static>
Vulkan_module
logger_module
Tracy::TracyClient
)
