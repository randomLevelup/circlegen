cmake_minimum_required(VERSION 3.5)

project(circlegen)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

find_package(PkgConfig REQUIRED)
pkg_check_modules(GLIB REQUIRED glib-2.0)
pkg_check_modules(CAIRO REQUIRED cairo)
pkg_check_modules(RSVG REQUIRED librsvg-2.0)

include_directories(
    include 
    /usr/local/include 
    /usr/include/eigen3 
    ${GLIB_INCLUDE_DIRS} 
    ${CAIRO_INCLUDE_DIRS} 
)

# Exclude the include/dummy/ directory
set(EXCLUDE_DIRS "include/dummy")

link_directories(./lib /usr/lib)

add_executable(circlegen cgparse.cpp cgproc.cpp cgfill.cpp cgrender.cpp main.cpp)

target_compile_options(circlegen PRIVATE -O2)
set_source_files_properties(cgfill.cpp PROPERTIES COMPILE_FLAGS -Wno-deprecated-declarations)
target_link_libraries(circlegen 
    png
    jpeg
    tinyxml2 
    ${GLIB_LIBRARIES} 
    ${CAIRO_LIBRARIES} 
)
target_link_options(circlegen PRIVATE -fopenmp -O2)