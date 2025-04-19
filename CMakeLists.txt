cmake_minimum_required(VERSION 3.16)

project(renderEngine)

set(CMAKE_CXX_STANDARD 20)

# Engine src
set(SRC "src/main.cpp" "src/renderEngine.cpp" "src/windowManager.cpp" "src/SceneManager.cpp" "src/geometry.cpp")
set(HEADER "include/renderEngine.h" "include/windowManager.h" "include/geometry.h" "include/common.h")

# vulkan module src
set(VULKAN_SRC "src/vulkanWrapper.cpp" "include/vulkanWrapper.h")

# d3d11 module src
set(D3D11_SRC "src/d3d11Wrapper.cpp" "include/d3d11Wrapper.h")

# logger module src
set(LOGGER_SRC "src/logger.cpp" "include/logger.h")

# required external libs
find_package(Vulkan REQUIRED)
find_package(glfw3 CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(SDL2 CONFIG REQUIRED)
find_package(Tracy CONFIG REQUIRED)
find_package(Stb REQUIRED)

# logger module library 
add_library(logger_module STATIC ${LOGGER_SRC})
target_link_libraries(logger_module PUBLIC)

# vulkan module library 
add_library(Vulkan_module STATIC ${VULKAN_SRC})
target_link_libraries(Vulkan_module PUBLIC glfw glm::glm Vulkan::Vulkan logger_module)

# d3d11 module library 
add_library(D3D11_module STATIC ${D3D11_SRC})
target_link_libraries(D3D11_module PUBLIC glm::glm d3d11.lib d3dcompiler.lib dxguid.lib)

# render engine exe.
add_executable(${PROJECT_NAME} ${SRC} ${HEADER})

target_link_libraries(${PROJECT_NAME}
$<TARGET_NAME_IF_EXISTS:SDL2::SDL2main> # SDL2
$<IF:$<TARGET_EXISTS:SDL2::SDL2>,SDL2::SDL2,SDL2::SDL2-static>
# Vulkan_module
D3D11_module
logger_module
Tracy::TracyClient
)

target_include_directories(${PROJECT_NAME} PRIVATE ${Stb_INCLUDE_DIR})

add_definitions(/arch:AVX2)