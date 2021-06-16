cmake_minimum_required(VERSION 3.5)

if(COMMAND cmake_policy)
  cmake_policy(SET CMP0003 NEW)
endif(COMMAND cmake_policy)

if(DEFINED ENV{VCPKG_ROOT} AND NOT DEFINED CMAKE_TOOLCHAIN_FILE)
    file(TO_CMAKE_PATH "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake" CMAKE_TOOLCHAIN_FILE)
endif()

project(raytracey)
add_subdirectory(hq)

add_executable(raytracey "")

target_sources(raytracey PRIVATE main.cpp
    BvhNode.h
    camera.h
    hitable.h
    sphere.h
    HitableList.h
    material.h
    Texture.h
    3rdParty/FastNoise/FastNoise.cpp
    3rdParty/FastNoise/FastNoise.h)

find_package(SDL2 REQUIRED)

target_include_directories(raytracey PRIVATE . 3rdParty)

target_link_libraries(raytracey hq SDL2::SDL2 SDL2::SDL2main)

target_compile_features(raytracey PUBLIC cxx_std_14)

add_custom_command(TARGET raytracey POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_CURRENT_SOURCE_DIR}/assets
    ${CMAKE_BINARY_DIR}/assets
    )

