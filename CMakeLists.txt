cmake_minimum_required(VERSION 3.30)
project(AlgoVisualizer LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD          23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_C_STANDARD            23)
set(CMAKE_C_STANDARD_REQUIRED   ON)

add_compile_options(-Wall -Wpedantic -Wextra -O3)

add_subdirectory(external)
add_subdirectory(src)

# web build (WASM)
if (EMSCRIPTEN)
    add_executable(AlgoVisualizer_wasm src/wasm_bindings.cpp)
    target_link_libraries(AlgoVisualizer_wasm PRIVATE Image)

    target_link_options(AlgoVisualizer_wasm PRIVATE
            "-lembind"
            "-sMODULARIZE=1"
            "-sEXPORT_NAME=WasmModule"
            "-sALLOW_MEMORY_GROWTH=1"
            "-sENVIRONMENT=web"
            "-O3"
    )

    set_target_properties(AlgoVisualizer_wasm PROPERTIES SUFFIX ".js")

# native build (CLI)
else()
    #add_compile_options(-fopenmp)
    #add_link_options(-fopenmp)
    #find_package(OpenMP REQUIRED)

    add_executable(AlgoVisualizer main.cpp)
    target_link_libraries(AlgoVisualizer PRIVATE Image)
endif()
