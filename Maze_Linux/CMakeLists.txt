cmake_minimum_required(VERSION 3.22)
project(maze_game)

set(CMAKE_CXX_STANDARD 17)

include_directories(.)
find_package(OpenGL REQUIRED)
find_package(GLUT REQUIRED)

add_executable(maze_game main.cpp)
target_link_libraries(maze_game ${OPENGL_LIBRARIES})
target_link_libraries(maze_game ${GLUT_LIBRARIES})
target_link_libraries(maze_game -lglut -lGL -lGLU)