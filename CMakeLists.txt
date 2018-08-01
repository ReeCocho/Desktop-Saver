cmake_minimum_required(VERSION 3.7)

# Project
project(Desktop-Saver)

# Includes
include_directories(${CMAKE_SOURCE_DIR}/dep)
include_directories(${CMAKE_SOURCE_DIR}/src)

# Executable
add_executable (
	Desktop-Saver 
	src/save_data.cpp
	src/save_data.hpp
	src/save_data.imp.hpp
	src/util.cpp
	src/util.hpp
	src/main.cpp
)