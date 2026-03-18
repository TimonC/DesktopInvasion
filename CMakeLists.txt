cmake_minimum_required(VERSION 3.16)
project(DesktopInvasion)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Quick)

qt_add_resources(RESOURCES resources.qrc)

add_executable(DesktopInvasion
    src/main.cpp
    src/InvasionOverlay.cpp
    ${RESOURCES}
)

target_include_directories(DesktopInvasion PRIVATE src)
target_link_libraries(DesktopInvasion Qt6::Core Qt6::Quick)
