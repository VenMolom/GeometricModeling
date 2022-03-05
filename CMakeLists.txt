cmake_minimum_required(VERSION 3.5)

project(MG1 VERSION 0.1 LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 COMPONENTS Widgets REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Widgets REQUIRED)

set(PROJECT_SOURCES
        main.cpp
        Window/mainwindow.cpp
        Window/mainwindow.h
        Window/mainwindow.ui
        Scene/scene.cpp
        Scene/scene.h
        Object/object.cpp
        Object/object.h
        Ellipsoid/ellipsoid.cpp
        Ellipsoid/ellipsoid.h
        Camera/camera.cpp
        Camera/camera.h
        Renderer/renderer.cpp
        Renderer/renderer.h
        Renderer/renderer.ui
        Controls/controls.h
        Controls/controls.cpp
        Controls/controls.ui)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(MG1
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
    )
# Define target properties for Android with Qt 6 as:
#    set_property(TARGET MG1 APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
#                 ${CMAKE_CURRENT_SOURCE_DIR}/android)
# For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation
else()
    if(ANDROID)
        add_library(MG1 SHARED
            ${PROJECT_SOURCES}
        )
# Define properties for Android with Qt 5 after find_package() calls as:
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
    else()
        add_executable(MG1
            ${PROJECT_SOURCES}
        )
    endif()
endif()

target_link_libraries(MG1 PRIVATE Qt${QT_VERSION_MAJOR}::Core)
target_link_libraries(MG1 PRIVATE Qt${QT_VERSION_MAJOR}::Gui)
target_link_libraries(MG1 PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)

set_target_properties(MG1 PROPERTIES
    MACOSX_BUNDLE_GUI_IDENTIFIER my.example.com
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(MG1)
endif()
