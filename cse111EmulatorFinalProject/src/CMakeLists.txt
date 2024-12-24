# Copyright (c) 2024 Ethan Sifferman.
# All rights reserved. Distribution Prohibited.

cmake_minimum_required(VERSION 3.14)

project(Banana)

set(CMAKE_CXX_STANDARD 17)

include_directories(.)

# Add executable

add_executable(${PROJECT_NAME}
    main.cpp
    cpu.cpp
    gpu.cpp
    console.cpp
)

add_executable(disassemble
    disassemble.cpp
    cpu.cpp
    gpu.cpp
    console.cpp
)

# Find SDL2
find_package(SDL2 REQUIRED)
include_directories(${SDL2_INCLUDE_DIRS})

# Link SDL2_image to targets
target_link_libraries(${PROJECT_NAME} PRIVATE ${SDL_IMAGE_LIBRARIES} SDL2_image)
target_link_libraries(disassemble PRIVATE ${SDL_IMAGE_LIBRARIES} SDL2_image)

include(FetchContent)
FetchContent_Declare(
    cli11_proj
    QUIET
    GIT_REPOSITORY https://github.com/CLIUtils/CLI11.git
    GIT_TAG v2.4.1
)

FetchContent_MakeAvailable(cli11_proj)

target_link_libraries(${PROJECT_NAME} PRIVATE CLI11::CLI11 ${SDL2_LIBRARIES})
target_link_libraries(disassemble PRIVATE CLI11::CLI11 ${SDL2_LIBRARIES})