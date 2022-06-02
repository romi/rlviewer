cmake_minimum_required(VERSION 3.16)
project(rl_viewer)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")

set(SOURCES
        src/main.cpp
        src/controls.cpp
        src/objloader.cpp
        src/shader.cpp
        src/texture.cpp
        src/vboindexer.cpp
        src/viewer.cpp)


add_executable(test_rlviewer ${SOURCES})
target_link_libraries(test_rlviewer GL glfw GLEW glm)
add_subdirectory(libs/glm-0.9.7.1)



