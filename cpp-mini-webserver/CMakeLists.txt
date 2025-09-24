cmake_minimum_required(VERSION 3.10)

project(cpp-mini-webserver LANGUAGES CXX)

# 设置 C++ 标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 指定可执行文件输出目录
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# 包含头文件目录
include_directories(${PROJECT_SOURCE_DIR}/include)

# 源文件列表
file(GLOB_RECURSE SOURCES
    ${PROJECT_SOURCE_DIR}/src/*.cpp
)

# 生成可执行文件
add_executable(cpp-mini-webserver ${SOURCES})

# 链接 pthread 库（多线程需要）
find_package(Threads REQUIRED)
target_link_libraries(cpp-mini-webserver PRIVATE Threads::Threads)
