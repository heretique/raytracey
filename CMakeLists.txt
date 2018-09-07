cmake_minimum_required(VERSION 3.5)

add_executable(raytracey "")

target_sources(raytracey PRIVATE main.cpp
    camera.h
    vector.h
    ray.h
    hitable.h
    sphere.h
    HitableList.h
    material.h
    JobManager.cpp
    JobManager.h
    utils.h
    utils.cpp)

find_package(SDL2 REQUIRED)

target_include_directories(raytracey PRIVATE .
    ${SDL2_INCLUDE_DIRS})

target_link_libraries(raytracey ${SDL2_LIBRARIES})

target_compile_features(raytracey PUBLIC cxx_std_14)
