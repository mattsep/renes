cmake_minimum_required(VERSION 3.11)

project(renes
    LANGUAGES CXX
    VERSION 0.1.0
)

option(RENES_ENABLE_LOGGING "Enable ReNES logging" ON)

set(CMAKE_CXX_EXTENSIONS OFF)

find_package(Threads REQUIRED)
find_package(wxWidgets REQUIRED COMPONENTS core base)

include(${wxWidgets_USE_FILE})

add_library(nes-lib INTERFACE)
target_compile_features(nes-lib INTERFACE cxx_std_17)
target_include_directories(nes-lib INTERFACE source)
target_compile_options(nes-lib INTERFACE
    $<$<OR:$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:AppleClang>,$<CXX_COMPILER_ID:GNU>>:
        -Wall -Wextra -pedantic-errors>
    $<$<CXX_COMPILER_ID:GNU>:
        -Wno-attributes>
    $<$<CXX_COMPILER_ID:MSVC>:
        /WX /W4>
)

add_library(gui-lib INTERFACE)
target_compile_features(gui-lib INTERFACE cxx_std_17)
target_include_directories(gui-lib INTERFACE source)
target_link_libraries(gui-lib INTERFACE nes-lib ${wxWidgets_LIBRARIES})

add_executable(renes source/renes.cpp)
target_link_libraries(renes PRIVATE nes-lib gui-lib Threads::Threads)

if(RENES_ENABLE_LOGGING)
    target_compile_definitions(renes PRIVATE -DRENES_ENABLE_LOGGING)
endif()