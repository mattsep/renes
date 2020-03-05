set(FILES
    Bus
    Cpu
    Nes
)

set(SOURCES "")
foreach(file ${FILES})
    list(APPEND SOURCES ${CMAKE_CURRENT_LIST_DIR}/renes/${file}.cpp)
endforeach()

add_library(renes-lib STATIC ${SOURCES})
target_include_directories(renes-lib PUBLIC ${CMAKE_SOURCE_DIR}/source)
target_compile_features(renes-lib PUBLIC cxx_std_17)

if(MSVC)
    target_compile_options(renes-lib PUBLIC
        /W4 $<$<CONFIG:RELEASE>:/02>
    )
else()
    target_compile_options(renes-lib PUBLIC
    -Wall -Wextra -pedantic
    $<$<CONFIG:RELEASE>:-O3 -DNDEBUG -flto>
    $<$<CONFIG:DEBUG>:-g -Og>
    )
endif()