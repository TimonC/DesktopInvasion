cmake_minimum_required(VERSION 3.16)
cmake_policy(SET CMP0100 NEW)

project(DesktopInvasion)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# Find Qt (adjust if you use Qt6 instead of Qt5)
find_package(Qt5 COMPONENTS Widgets REQUIRED)

add_executable(DesktopInvasion
    src/main.cc
    src/InvasionView.cc
)

target_include_directories(DesktopInvasion PRIVATE src)
target_link_libraries(DesktopInvasion Qt5::Widgets)

