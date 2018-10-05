cmake_minimum_required(VERSION 3.5)

add_executable(raytracey "")

target_sources(raytracey PRIVATE main.cpp
    aabb.h
    BvhNode.h
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

if (${CMAKE_SYSTEM_NAME} MATCHES "Windows")
    set(SDL2_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/SDL2/include)
    set(SDL2_LIB_PREFIX ${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/SDL2/msvc/x64)
    set(SDL2_LIBRARIES ${SDL2_LIB_PREFIX}/SDL2main.lib ${SDL2_LIB_PREFIX}/SDL2.lib)
else()
    find_package(SDL2 REQUIRED)
endif()

target_include_directories(raytracey PRIVATE .
    ${SDL2_INCLUDE_DIRS})

target_link_libraries(raytracey ${SDL2_LIBRARIES})

target_compile_features(raytracey PUBLIC cxx_std_14)
