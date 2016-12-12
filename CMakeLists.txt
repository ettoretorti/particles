cmake_minimum_required (VERSION 3.1)
project (particles)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(OpenGL REQUIRED)

set(mathfu_build_benchmarks OFF CACHE BOOL "" FORCE)
set(mathfu_build_tests OFF CACHE BOOL "" FORCE)
add_subdirectory(${CMAKE_SOURCE_DIR}/deps/mathfu)

set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
add_subdirectory(${CMAKE_SOURCE_DIR}/deps/glfw)

file(GLOB_RECURSE SOURCE_FILES "src/*.cpp" "src/*.c")
#source groups split files into the correct hierarchy in VS
source_group(src
    REGULAR_EXPRESSION ${CMAKE_SOURCE_DIR}/src/*)
source_group(src\\geom
    REGULAR_EXPRESSION ${CMAKE_SOURCE_DIR}/src/geom/*)
source_group(src\\glh
    REGULAR_EXPRESSION ${CMAKE_SOURCE_DIR}/src/glh/*)

add_executable(particles ${SOURCE_FILES})

target_include_directories(particles
    PRIVATE ${CMAKE_SOURCE_DIR}/src
    PRIVATE ${CMAKE_SOURCE_DIR}/include)

if (NOT "${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
    #g++ options
    target_compile_options(particles
        PRIVATE -Wall
        PRIVATE -Wextra
        PRIVATE -pedantic)
endif()

mathfu_configure_flags(particles)

target_link_libraries(particles
    ${OPENGL_LIBRARIES}
    glfw
    dl)
