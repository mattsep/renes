set(FILES
    Bus
    Cpu
    Nes
)

set(SOURCES "")
foreach(file ${FILES})
    list(APPEND SOURCES ${CMAKE_CURRENT_LIST_DIR}/${file}.cpp)
endforeach()

add_library(renes-lib STATIC ${SOURCES})
target_include_directories(renes-lib PUBLIC ${CMAKE_SOURCE_DIR}/source)