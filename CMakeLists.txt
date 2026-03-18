cmake_minimum_required(VERSION 3.16)
project(DesktopInvasion LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Quick Widgets)
qt6_add_resources(RESOURCES_RCC resources.qrc)

file(GLOB_RECURSE ALL_SRC CONFIGURE_DEPENDS src/*.cpp)
file(GLOB_RECURSE ALL_HEADERS CONFIGURE_DEPENDS include/*.h src/*.h)

add_executable(DesktopInvasion ${ALL_SRC} ${ALL_HEADERS} ${RESOURCES_RCC})
target_include_directories(DesktopInvasion PRIVATE ${CMAKE_SOURCE_DIR}/include)
target_link_libraries(DesktopInvasion PRIVATE Qt6::Core Qt6::Gui Qt6::Quick Qt6::Widgets sqlite3)
