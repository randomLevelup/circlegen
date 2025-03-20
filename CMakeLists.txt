cmake_minimum_required(VERSION 3.16)
project(QtImageApp LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Qt5 configuration
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(Qt5 REQUIRED COMPONENTS Core Widgets Gui)

include_directories(
    include 
    /usr/local/include 
    /usr/include/eigen3 
    /usr/include/x86_64-linux-gnu/qt5
)

# Add executable
add_executable(image_app main.cpp)

target_include_directories(image_app PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

# Link Qt libraries
target_link_libraries(image_app
    PRIVATE 
    Qt5::Core
    Qt5::Widgets
    Qt5::Gui
)