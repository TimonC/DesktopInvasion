cmake_minimum_required(VERSION 3.16)
project(DesktopInvasion)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC OFF)   # We'll manually add resources

find_package(Qt6 REQUIRED COMPONENTS Core Gui Quick)

# Compile the Qt resource file
qt6_add_resources(RESOURCES_RCC
    resources.qrc
)

# Add executable
add_executable(DesktopInvasion
    src/PokemonInteractable.cpp
    src/main.cpp
    ${RESOURCES_RCC}
)

# Include headers
target_include_directories(DesktopInvasion PRIVATE src)

# Link Qt libraries
target_link_libraries(DesktopInvasion
    Qt6::Core
    Qt6::Gui
    Qt6::Quick
)

