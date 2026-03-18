cmake_minimum_required(VERSION 3.16)
cmake_policy(SET CMP0100 NEW)

project(DesktopInvasion)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt5 COMPONENTS Widgets REQUIRED)

add_executable(DesktopInvasion
    src/main.cpp
    src/InvasionView.cpp
)

target_include_directories(DesktopInvasion PRIVATE src)
target_link_libraries(DesktopInvasion Qt5::Widgets)

