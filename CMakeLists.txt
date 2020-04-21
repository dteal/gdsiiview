cmake_minimum_required(VERSION 3.10)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_executable(gdsiiview src/main.cpp)
target_link_libraries(gdsiiview glfw)
target_link_libraries(gdsiiview GL)
target_link_libraries(gdsiiview GLEW)
target_link_libraries(gdsiiview boost_system)
target_link_libraries(gdsiiview boost_filesystem)