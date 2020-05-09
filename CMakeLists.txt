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

set(SOURCES
    bus.cpp
    cartridge.cpp
    console.cpp
    cpu.cpp
    display.cpp
    ppu.cpp
    mappers.cpp
    mappers/mapper_000.cpp
)

set(NES_SOURCE_FILES "")
foreach(FILE ${SOURCES})
    list(APPEND NES_SOURCE_FILES source/nes/${FILE})
endforeach()

add_library(nes-lib STATIC ${NES_SOURCE_FILES})
target_compile_features(nes-lib PUBLIC cxx_std_17)
target_include_directories(nes-lib PUBLIC source)
target_compile_options(nes-lib PUBLIC
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
    target_compile_definitions(nes-lib PUBLIC -DRENES_ENABLE_LOGGING)
endif()