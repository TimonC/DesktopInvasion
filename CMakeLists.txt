cmake_minimum_required(VERSION 3.16)
project(DesktopInvasion)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)  # <-- tells CMake to generate compile_commands.json
set(CMAKE_AUTOMOC ON)                  # <-- needed for Q_OBJECT macro
# Find Qt5 Widgets library
find_package(Qt5 COMPONENTS Widgets REQUIRED)

# Add your source files
add_executable(DesktopInvasion
    src/main.cc
    src/InvasionView.cc
)

# Link Qt5 Widgets library
target_link_libraries(DesktopInvasion Qt5::Widgets)
