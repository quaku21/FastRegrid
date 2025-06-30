cmake_minimum_required(VERSION 3.10)
project(FastRegrid LANGUAGES CXX)

# Set C++ standard (C++17 minimum, compatible with C++17, C++20, C++23)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Add subdirectories
add_subdirectory(include/fastregrid)
add_subdirectory(examples)