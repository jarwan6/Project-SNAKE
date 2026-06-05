cmake_minimum_required(VERSION 3.20)
project(SnakeArenaGame VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ── SFML ──────────────────────────────────────────────────────────────────────
find_package(SFML 3.1 COMPONENTS Graphics Window System REQUIRED)

# ── Main executable ───────────────────────────────────────────────────────────
add_executable(SnakeArena src/main.cpp)

target_link_libraries(SnakeArena
    PRIVATE
        SFML::Graphics
        SFML::Window
        SFML::System
)

# Copy assets next to the binary after build
foreach(ASSET menu.png background.png arial.ttf)
    if(EXISTS "${CMAKE_SOURCE_DIR}/assets/${ASSET}")
        add_custom_command(TARGET SnakeArena POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${CMAKE_SOURCE_DIR}/assets/${ASSET}"
                "$<TARGET_FILE_DIR:SnakeArena>/${ASSET}"
        )
    endif()
endforeach()

# ── Tests ─────────────────────────────────────────────────────────────────────
enable_testing()

add_executable(TestSnakeLogic tests/test_logic.cpp)

target_include_directories(TestSnakeLogic PRIVATE tests)

# Tests are pure C++ — no SFML needed
add_test(NAME SnakeLogicTests COMMAND TestSnakeLogic)
