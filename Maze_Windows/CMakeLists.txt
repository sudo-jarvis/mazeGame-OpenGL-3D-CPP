cmake_minimum_required(VERSION 3.23)
project(maze1)

set(CMAKE_CXX_STANDARD 14)

include_directories(.)
find_package(OpenGL REQUIRED)
find_package(GLUT REQUIRED)

add_executable(maze1 main.cpp)
target_link_libraries(maze1 ${OPENGL_LIBRARIES})
target_link_libraries(maze1 ${GLUT_LIBRARIES})
target_link_libraries(maze1 -lopengl32 -lwinmm)