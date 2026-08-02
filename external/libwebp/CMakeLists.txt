file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS
        "${CMAKE_CURRENT_SOURCE_DIR}/src/*.c"
        "${CMAKE_CURRENT_SOURCE_DIR}/sharpyuv/*.c"
)

add_library(LibWebP STATIC ${SOURCES})

target_include_directories(LibWebP PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        ${CMAKE_CURRENT_SOURCE_DIR}/sharpyuv
        ${CMAKE_CURRENT_SOURCE_DIR}
)
