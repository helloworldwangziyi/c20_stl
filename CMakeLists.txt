cmake_minimum_required(VERSION 3.20)

project(cpp20_stl LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# 头文件库 c20_stl
add_library(c20_stl INTERFACE)

# 库文件来源
target_include_directories(c20_stl INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/src")

enable_testing()

# 每加一个测试,只需在列表里加一行文件名(不含 .cpp)
set(TEST_SOURCES
    smoke_test
    test_type_traits
)

foreach(name IN LISTS TEST_SOURCES)
    add_executable(c20_${name} tests/${name}.cpp)
    target_link_libraries(c20_${name} PRIVATE c20_stl)
    add_test(NAME ${name} COMMAND c20_${name})
endforeach()

