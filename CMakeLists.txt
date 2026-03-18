cmake_minimum_required(VERSION 3.16)
project(DesktopInvasion LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)
set(BUILD_SHARED_LIBS ON)

# Find Qt6 - let CMake handle the paths automatically
# On GitHub Actions, Qt6_DIR is set by the install-qt-action
# On local Docker builds, CMake will find it via CMAKE_PREFIX_PATH or default paths
find_package(Qt6 REQUIRED COMPONENTS Core Gui Quick Widgets Sql)

# Add Qt resources
qt6_add_resources(RESOURCES_RCC resources.qrc)

# Source files
file(GLOB_RECURSE ALL_SRC CONFIGURE_DEPENDS src/*.cpp)
file(GLOB_RECURSE ALL_HEADERS CONFIGURE_DEPENDS include/*.h src/*.h)

# Windows-specific settings
if(WIN32)
    # Set Windows subsystem (no console window)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -mwindows")

    # Handle application icon
    set(ICON_FILE "${CMAKE_SOURCE_DIR}/assets/icon/icon.ico")
    if(EXISTS ${ICON_FILE})
        set(RC_FILE "${CMAKE_CURRENT_BINARY_DIR}/app.rc")
        file(WRITE ${RC_FILE} "IDI_ICON1 ICON \"${ICON_FILE}\"\n")
        list(APPEND ALL_SRC ${RC_FILE})

        # Enable RC compiler (for .ico files)
        set(CMAKE_RC_COMPILER_INIT windres)
        enable_language(RC)
    else()
        message(WARNING "Icon file not found: ${ICON_FILE}")
    endif()
endif()

# macOS-specific settings
if(APPLE)
    # Set macOS bundle properties
    set_target_properties(DesktopInvasion PROPERTIES
        MACOSX_BUNDLE TRUE
        MACOSX_BUNDLE_BUNDLE_NAME "DesktopInvasion"
        MACOSX_BUNDLE_BUNDLE_VERSION "1.0.0"
        MACOSX_BUNDLE_SHORT_VERSION_STRING "1.0.0"
        MACOSX_BUNDLE_INFO_PLIST "${CMAKE_CURRENT_SOURCE_DIR}/macos/Info.plist"
    )

    # Handle application icon for macOS
    set(ICON_FILE "${CMAKE_SOURCE_DIR}/assets/icon/icon.icns")
    if(EXISTS ${ICON_FILE})
        set_source_files_properties(${ICON_FILE} PROPERTIES
            MACOSX_PACKAGE_LOCATION "Resources"
        )
        list(APPEND ALL_SRC ${ICON_FILE})
    else()
        message(WARNING "Icon file not found: ${ICON_FILE}")
    endif()
endif()

# Create executable
add_executable(DesktopInvasion ${ALL_SRC} ${ALL_HEADERS} ${RESOURCES_RCC})

# Precompile headers for faster builds
target_precompile_headers(DesktopInvasion PRIVATE
    <QtCore>
    <QtGui>
    <QtQuick>
    <QtWidgets>
    <QtSql>
)

# Include directories
target_include_directories(DesktopInvasion PRIVATE
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_CURRENT_BINARY_DIR}
)

# Link Qt libraries
target_link_libraries(DesktopInvasion PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Quick
    Qt6::Widgets
    Qt6::Sql
)

# Set output directory
set_target_properties(DesktopInvasion PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

# Optional: Add version information
if(EXISTS "${CMAKE_SOURCE_DIR}/VERSION")
    file(READ "${CMAKE_SOURCE_DIR}/VERSION" VERSION_STRING)
    string(STRIP "${VERSION_STRING}" VERSION_STRING)
    set_target_properties(DesktopInvasion PROPERTIES
        VERSION "${VERSION_STRING}"
        OUTPUT_NAME "DesktopInvasion"
    )
endif()

# Print build configuration
message(STATUS "Building DesktopInvasion")
message(STATUS "  Qt version: ${Qt6_VERSION}")
message(STATUS "  Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "  Install prefix: ${CMAKE_INSTALL_PREFIX}")
message(STATUS "  Platform: ${CMAKE_SYSTEM_NAME}")
