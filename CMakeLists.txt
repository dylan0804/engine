cmake_minimum_required(VERSION 4.3)
project(Test)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(glfw3 REQUIRED)

add_executable(app 
  src/main.cpp
  src/glad.c
)

target_include_directories(app PRIVATE include)
target_link_libraries(app PRIVATE glfw dl)

