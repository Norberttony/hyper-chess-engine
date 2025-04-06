cmake_minimum_required(VERSION 3.10.0)
project(Hyper-Active VERSION 0.1.0 LANGUAGES C)

# Set the C standard
set(CMAKE_C_STANDARD 11)

# Set compiler flags for all targets
add_compile_options(
    -Wall
    -Wpedantic
    -Werror
    -Wno-parentheses
    -O3
    -ffast-math
)

include_directories(src)

# Set source files
set(SOURCES
    src/hyper-active.c
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

# WASM-specific source files (uses wasm-main.c)
set(WASM_SOURCES
    src/wasm-main.c
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

# Output binary in bin/
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)

# Native build (default unless emscripten is used)
if(NOT EMSCRIPTEN)
    message(STATUS "Configuring native build")

    add_executable(hyper-active ${SOURCES})
else()
    message(STATUS "Configuring WebAssembly build")

    # .js output (instead of .wasm or no extension)
    set(CMAKE_EXECUTABLE_SUFFIX ".js")

    add_executable(hyper-active ${WASM_SOURCES})

    target_compile_definitions(hyper-active PRIVATE WEB)

    set_target_properties(hyper-active PROPERTIES
        LINK_FLAGS "-s NO_EXIT_RUNTIME=1 -s EXPORTED_RUNTIME_METHODS=[ccall]"
    )
endif()
