cmake_minimum_required(VERSION 3.11)

project(renes
    LANGUAGES CXX
    VERSION 0.1.0
)

add_subdirectory(source)

add_executable(renes source/main.cpp)
target_link_libraries(renes PRIVATE renes-lib)