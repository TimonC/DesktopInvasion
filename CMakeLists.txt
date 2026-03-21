cmake_minimum_required(VERSION 3.16)

project(DesktopInvasion LANGUAGES CXX OBJCXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Quick Widgets Sql)

qt6_add_resources(RESOURCES_RCC resources.qrc)

file(GLOB_RECURSE CPP_SRC CONFIGURE_DEPENDS src/*.cpp)
file(GLOB_RECURSE ALL_HEADERS CONFIGURE_DEPENDS include/*.h src/*.h)
file(GLOB_RECURSE MM_SRC CONFIGURE_DEPENDS src/*.mm)

set(ALL_SRC ${CPP_SRC} ${ALL_HEADERS} ${RESOURCES_RCC})

add_executable(DesktopInvasion
    ${ALL_SRC}
    ${MM_SRC}
)

set_target_properties(DesktopInvasion PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

if(APPLE)
    set_source_files_properties(${MM_SRC} PROPERTIES
        LANGUAGE OBJCXX
        COMPILE_FLAGS "-fobjc-arc"
    )

    set_target_properties(DesktopInvasion PROPERTIES
        MACOSX_BUNDLE                   TRUE
        MACOSX_BUNDLE_BUNDLE_NAME       "DesktopInvasion"
        MACOSX_BUNDLE_BUNDLE_VERSION    "1.0.0"
        MACOSX_BUNDLE_SHORT_VERSION_STRING "1.0.0"
        MACOSX_BUNDLE_INFO_PLIST        "${CMAKE_CURRENT_SOURCE_DIR}/macos/Info.plist"
    )

    set(ICON_FILE "${CMAKE_SOURCE_DIR}/assets/icon/icon.icns")
    if(EXISTS ${ICON_FILE})
        set_source_files_properties(${ICON_FILE} PROPERTIES
            MACOSX_PACKAGE_LOCATION "Resources"
        )
        target_sources(DesktopInvasion PRIVATE ${ICON_FILE})
    else()
        message(WARNING "Icon file not found: ${ICON_FILE}")
    endif()

    # Link macOS frameworks
    target_link_libraries(DesktopInvasion PRIVATE
        "-framework Cocoa"
        "-framework Foundation"
        "-framework AppKit"
    )
endif()

if(WIN32)
    set_target_properties(DesktopInvasion PROPERTIES
        WIN32_EXECUTABLE TRUE
    )

    add_custom_command(TARGET DesktopInvasion POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_RUNTIME_DLLS:DesktopInvasion>
            $<TARGET_FILE_DIR:DesktopInvasion>
        COMMAND_EXPAND_LISTS
    )

    set(ICON_FILE "${CMAKE_SOURCE_DIR}/assets/icon/icon.ico")
    if(EXISTS ${ICON_FILE})
        set(RC_FILE "${CMAKE_CURRENT_BINARY_DIR}/app.rc")
        file(WRITE ${RC_FILE} "IDI_ICON1 ICON \"${ICON_FILE}\"\n")
        target_sources(DesktopInvasion PRIVATE ${RC_FILE})
        enable_language(RC)
    else()
        message(WARNING "Icon file not found: ${ICON_FILE}")
    endif()
endif()

target_precompile_headers(DesktopInvasion PRIVATE
    <QtCore>
    <QtGui>
    <QtQuick>
    <QtWidgets>
    <QtSql>
)

target_include_directories(DesktopInvasion PRIVATE
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_CURRENT_BINARY_DIR}
)

target_link_libraries(DesktopInvasion PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Quick
    Qt6::Widgets
    Qt6::Sql
)

message(STATUS "Building DesktopInvasion")
message(STATUS "  Qt version:     ${Qt6_VERSION}")
message(STATUS "  Build type:     ${CMAKE_BUILD_TYPE}")
message(STATUS "  Install prefix: ${CMAKE_INSTALL_PREFIX}")
