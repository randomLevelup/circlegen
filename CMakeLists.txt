cmake_minimum_required(VERSION 3.5)

project(circlegen)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt5 COMPONENTS Widgets REQUIRED)
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
    ${RSVG_INCLUDE_DIRS}
)

link_directories(./lib /usr/lib)

add_executable(circlegen cgio.cpp cgproc.cpp main.cpp)

target_link_libraries(circlegen 
    Qt5::Widgets 
    tinyxml2 
    ${GLIB_LIBRARIES} 
    ${CAIRO_LIBRARIES} 
    ${RSVG_LIBRARIES}
)
target_link_options(circlegen PRIVATE -fopenmp)