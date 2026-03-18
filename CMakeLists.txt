cmake_minimum_required(VERSION 3.16)
project(DesktopInvasion)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC OFF)   # Turn off automatic resources

find_package(Qt6 REQUIRED COMPONENTS Core Quick)

add_executable(DesktopInvasion
    src/main.cpp
    src/InvasionOverlay.cpp
)

target_include_directories(DesktopInvasion PRIVATE src)
target_link_libraries(DesktopInvasion
    Qt6::Core
    Qt6::Gui
    Qt6::Quick
)

