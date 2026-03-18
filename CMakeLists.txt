cmake_minimum_required(VERSION 3.16)
cmake_policy(SET CMP0100 NEW)

project(DesktopInvasion)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt5 COMPONENTS Widgets REQUIRED)

qt5_add_resources(RESOURCES resources.qrc)

add_executable(DesktopInvasion
    src/main.cpp
    src/InvasionView.cpp
    src/Sprite.cpp
    ${RESOURCES}
)
target_include_directories(DesktopInvasion PRIVATE src)
target_link_libraries(DesktopInvasion Qt5::Widgets)

