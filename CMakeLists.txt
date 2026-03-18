cmake_minimum_required(VERSION 3.16)
project(DesktopInvasion LANGUAGES CXX)

# Enable Qt automatic processing
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC OFF)   # We'll manually add resources

# Require Qt6
find_package(Qt6 REQUIRED COMPONENTS Gui Quick Widgets)

# Add resources
qt6_add_resources(RESOURCES_RCC resources.qrc)

# Automatically find all source and header files
file(GLOB_RECURSE SRC_FILES CONFIGURE_DEPENDS src/*.cpp)
file(GLOB_RECURSE HEADER_FILES CONFIGURE_DEPENDS include/*.h include/*.hpp)

# Create the executable target
add_executable(DesktopInvasion
    ${SRC_FILES}
    ${HEADER_FILES}
    ${RESOURCES_RCC}
)

# Include directories (headers live in include/, code in src/)
target_include_directories(DesktopInvasion PRIVATE
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/src
)

# Set the C++ standard
target_compile_features(DesktopInvasion PRIVATE cxx_std_17)

# Link Qt libraries
target_link_libraries(DesktopInvasion PRIVATE
    Qt6::Gui
    Qt6::Quick
    Qt6::Widgets
)

