cmake_minimum_required(VERSION 3.20)

project(cpp20_stl LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# 头文件库 c20_stl
add_library(c20_stl INTERFACE)

# 库文件来源
target_include_directories(c20_stl INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/src")

add_executable(c20_stl_smoke_test tests/smoke_test.cpp)


target_link_libraries(c20_stl_smoke_test PRIVATE c20_stl)
