cmake_minimum_required(VERSION 3.50)
project(Hyper-Active VERSION 0.1.0 LANGUAGES C)

# Set the C standard
set(CMAKE_C_STANDARD 11)

if(MSVC)
    # compiler options for MSVC
    add_compile_options(
        /O2
        /W4
        /std:c11
    )
else()
    # compiler options for gcc and clang
    add_compile_options(
        -Wall
        -Wpedantic
        -Werror
        -Wno-parentheses
        -O3
        -ffast-math
    )
endif()

# Include the header files
include_directories(src)

# ENGINE_SOURCES will be used for all builds of the engine,
# with an entry point (likely containing main) appended based on the build details.
set(ENGINE_SOURCES
    src/bitboard-utility.c
    src/defines.c
    src/magic-bitboards.c
    src/move.c
    src/perft.c
    src/look-up-tables.c
    src/test-suite.c
    src/evaluate.c
    src/transposition-table.c
    src/move-ordering.c
    src/think.c
    src/game-analysis.c
    src/make-unmake.c
    src/evaluate-defines.c
    src/uci.c
    src/utils.c
)

# Native build (default unless emscripten is used)
if(NOT EMSCRIPTEN)
    message(STATUS "Configuring native build")

    # add the file containing the main function
    list(APPEND ENGINE_SOURCES "src/hyper-active.c")

    add_executable(hyper-active ${ENGINE_SOURCES})
else()
    message(STATUS "Configuring WebAssembly build")

    # .js output (instead of .wasm or no extension)
    set(CMAKE_EXECUTABLE_SUFFIX ".js")

    # add the file containing the main function
    list(APPEND ENGINE_SOURCES "src/wasm-main.c")

    add_executable(wasm-build ${ENGINE_SOURCES})

    target_compile_definitions(wasm-build PRIVATE WEB)

    set_target_properties(wasm-build PROPERTIES
        LINK_FLAGS "-s NO_EXIT_RUNTIME=1 -s EXPORTED_RUNTIME_METHODS=[ccall]"
    )
endif()
