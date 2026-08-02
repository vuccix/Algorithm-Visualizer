add_library(External OBJECT ext.cpp)

target_include_directories(External PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

add_subdirectory(libwebp)

target_link_libraries(External
        PUBLIC
        LibWebP
)
