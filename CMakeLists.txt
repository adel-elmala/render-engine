cmake_minimum_required(VERSION 3.16)

project(renderEngine)

set(CMAKE_CXX_STANDARD 17)

set(SRC "src/main.cpp" "src/renderEngine.cpp" "src/windowManager.cpp" "src/Application.cpp")
set(HEADER "include/renderEngine.h" "include/windowManager.h")

find_package(SDL2 CONFIG REQUIRED)

find_package(glm CONFIG REQUIRED)

add_executable(${PROJECT_NAME} ${SRC})

target_link_libraries(${PROJECT_NAME}
$<TARGET_NAME_IF_EXISTS:SDL2::SDL2main>
$<IF:$<TARGET_EXISTS:SDL2::SDL2>,SDL2::SDL2,SDL2::SDL2-static>
glm::glm
)
