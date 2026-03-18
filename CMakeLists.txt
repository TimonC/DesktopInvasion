cmake_minimum_required(VERSION 3.16)
project(DesktopInvasion LANGUAGES CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC OFF)   # We'll manually add resources


find_package(Qt6 REQUIRED COMPONENTS Core Gui Quick)

# Create executable
add_executable(DesktopInvasion)

# Add source files
qt6_add_resources(RESOURCES_RCC resources.qrc)
target_sources(DesktopInvasion PRIVATE
    src/main.cpp
    src/PokemonInteractable.cpp
    ${RESOURCES_RCC}
)

# Include directories
target_include_directories(DesktopInvasion PRIVATE src)

# Set compile features
target_compile_features(DesktopInvasion PRIVATE cxx_std_17)

# Link Qt libraries
target_link_libraries(DesktopInvasion PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Quick
)

