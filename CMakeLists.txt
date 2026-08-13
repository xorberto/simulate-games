cmake_minimum_required(VERSION 3.15)
project(simulate-games LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Use the static MSVC runtime (/MT, /MTd)
if(MSVC)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif()

# Fallback to Release if no build type is specified
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

include(FetchContent)
FetchContent_Declare(json URL https://github.com/nlohmann/json/releases/download/v3.12.0/json.tar.xz DOWNLOAD_EXTRACT_TIMESTAMP true)
FetchContent_MakeAvailable(json)

# Dummy game executable
add_executable(dummy_game src/dummy_game.cpp src/core/terminal.cpp src/core/theme.cpp)
target_include_directories(dummy_game PRIVATE include)
set_target_properties(dummy_game PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/intermediaries")

# Strip symbols and optimize for size
if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    target_compile_options(dummy_game PRIVATE -Os)
    target_link_options(dummy_game PRIVATE -s)
endif()

# Static linking for Windows (GCC/Clang)
if((WIN32 OR MINGW) AND (CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang"))
    target_link_options(dummy_game PRIVATE -static -static-libgcc -static-libstdc++)
endif()

# Binary embedding for the launcher
if(WIN32 OR MINGW)
    # Windows: Embed using a Native Resource File (.rc)
    file(GENERATE OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/intermediaries/$<CONFIG>/payload.rc"
        CONTENT "101 RCDATA \"$<TARGET_FILE:dummy_game>\"\n"
    )
    set(LAUNCHER_RESOURCES "${CMAKE_CURRENT_BINARY_DIR}/intermediaries/$<CONFIG>/payload.rc")
else()
    # Linux: Embed using a GNU Assembly .incbin directive
    enable_language(ASM)
    file(GENERATE OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/intermediaries/$<CONFIG>/payload.S"
        CONTENT ".global payload_data\n.global payload_size\n.section .rodata\npayload_data:\n    .incbin \"$<TARGET_FILE:dummy_game>\"\n1:\n    .align 8\npayload_size:\n    .quad 1b - payload_data\n"
    )
    set(LAUNCHER_RESOURCES "${CMAKE_CURRENT_BINARY_DIR}/intermediaries/$<CONFIG>/payload.S")
endif()

# Launcher
add_executable(launcher src/launcher.cpp src/core/search.cpp src/core/terminal.cpp src/core/theme.cpp ${LAUNCHER_RESOURCES})
target_include_directories(launcher PRIVATE include)
set_target_properties(launcher PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIG>")

target_link_libraries(launcher PRIVATE nlohmann_json::nlohmann_json)
add_dependencies(launcher dummy_game)

# Strip symbols and optimize for size
if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    target_compile_options(launcher PRIVATE -Os)
    target_link_options(launcher PRIVATE -s)
endif()

# Static linking for Windows (GCC/Clang)
if((WIN32 OR MINGW) AND (CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang"))
    target_link_options(launcher PRIVATE -static -static-libgcc -static-libstdc++)
endif()

# Copy info.json to folder the executable was built in
add_custom_command(TARGET launcher POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "${CMAKE_CURRENT_SOURCE_DIR}/info.json"
    $<TARGET_FILE_DIR:launcher>/info.json
)

# Sync compile_commands.json
add_custom_target(sync_compile_commands ALL
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_BINARY_DIR}/compile_commands.json"
        "${CMAKE_SOURCE_DIR}/build/compile_commands.json"
    COMMENT "Syncing compile_commands.json"
)