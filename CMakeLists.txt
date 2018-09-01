cmake_minimum_required(VERSION 3.5)

add_executable(raytracey "")

target_sources(raytracey PRIVATE main.cpp
    vector.h
    ray.h
    JobManager.cpp
    JobManager.h)

find_package(SDL2 REQUIRED)

target_include_directories(raytracey PRIVATE .
    ${SDL2_INCLUDE_DIRS})

target_link_libraries(raytracey ${SDL2_LIBRARIES})

target_compile_features(raytracey PUBLIC cxx_std_14)