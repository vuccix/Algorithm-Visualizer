add_library(Image STATIC
        Image.cpp
        Impl/Sort.cpp
        Impl/AlgoCarving.cpp
        Impl/Utils.cpp
)

target_include_directories(Image PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(Image
        PRIVATE
        External
#        OpenMP::OpenMP_CXX
)
