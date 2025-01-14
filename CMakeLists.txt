cmake_minimum_required(VERSION 3.5)

project(circlegen)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt5 COMPONENTS Widgets REQUIRED)

include_directories(include /usr/local/include /usr/include/eigen3)

link_directories(./lib /usr/lib /usr/local/lib/Matplot++)

add_executable(circlegen cgio.cpp cgproc.cpp main.cpp)

target_link_libraries(circlegen Qt5::Widgets tinyxml2 matplot nodesoup)
target_link_options(circlegen PRIVATE -fopenmp)