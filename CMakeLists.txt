cmake_minimum_required(VERSION 3.5)

project(MG1 VERSION 0.1 LANGUAGES CXX)

ADD_DEFINITIONS(-DUNICODE)
ADD_DEFINITIONS(-D_UNICODE)
ADD_DEFINITIONS(-D_ENABLE_EXTENDED_ALIGNED_STORAGE)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 COMPONENTS Widgets REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Widgets REQUIRED)
find_library(D3D11_LIBRARY d3d11.lib)

set(PROJECT_SOURCES
        main.cpp
        Window/mainwindow.cpp Window/mainwindow.h Window/mainwindow.ui
        Scene/scene.cpp Scene/scene.h
        Objects/Object/object.cpp Objects/Object/object.h
        Camera/camera.cpp Camera/camera.h
        Controls/controls.h Controls/controls.cpp Controls/controls.ui
        DirectX/DXRenderer/dxRenderer.cpp DirectX/DXRenderer/dxRenderer.h
        DirectX/DXDevice/dxDevice.cpp DirectX/DXDevice/dxDevice.h
        DirectX/DXDevice/dxptr.h
        DirectX/DXStructures/dxStructures.cpp DirectX/DXStructures/dxStructures.h
        DirectX/DXDevice/exceptions.cpp DirectX/DXDevice/exceptions.h
        Renderer/Renderer.h
        Objects/ParametricObject/parametricObject.h
        Objects/Torus/torus.cpp Objects/Torus/torus.h
        Resources.qrc Handlers/inputHandler.cpp Handlers/inputHandler.h
        Objects/Cursor/cursor.cpp Objects/Cursor/cursor.h
        Utils/Utils3D.h Utils/Utils3D.cpp
        Objects/Point/point.cpp Objects/Point/point.h
        Objects/ObjectListItem/objectListItem.cpp Objects/ObjectListItem/objectListItem.h
        Objects/CompositeObject/compositeObject.cpp Objects/CompositeObject/compositeObject.h
        Objects/Brezier/brezierC0.cpp Objects/Brezier/brezierC0.h
        Objects/Brezier/brezierCurve.cpp Objects/Brezier/brezierCurve.h
        Objects/Brezier/brezierC2.cpp Objects/Brezier/brezierC2.h
        Controls/Modules/ObjectModule/objectmodule.h Controls/Modules/ObjectModule/objectmodule.cpp
        Controls/Modules/ObjectModule/objectmodule.ui
        Controls/Modules/ParametricModule/parametricmodule.h Controls/Modules/ParametricModule/parametricmodule.cpp
        Controls/Modules/ParametricModule/parametricmodule.ui
        Controls/Modules/TorusModule/torusmodule.h Controls/Modules/TorusModule/torusmodule.cpp
        Controls/Modules/TorusModule/torusmodule.ui
        Controls/Modules/BrezierCurveModule/breziercurvemodule.h Controls/Modules/BrezierCurveModule/breziercurvemodule.cpp
        Controls/Modules/BrezierCurveModule/breziercurvemodule.ui
        Controls/Modules/MoveModule/movemodule.h Controls/Modules/MoveModule/movemodule.cpp
        Controls/Modules/MoveModule/movemodule.ui
        Controls/Modules/TransformModule/transformmodule.h Controls/Modules/TransformModule/transformmodule.cpp
        Controls/Modules/TransformModule/transformmodule.ui
        Controls/Modules/ScreenMoveModule/screenmovemodule.h Controls/Modules/ScreenMoveModule/screenmovemodule.cpp
        Controls/Modules/ScreenMoveModule/screenmovemodule.ui
        Controls/Modules/BSplineModule/bsplinemodule.h Controls/Modules/BSplineModule/bsplinemodule.cpp
        Controls/Modules/BSplineModule/bsplinemodule.ui Objects/VirtualPoint/virtualPoint.cpp Objects/VirtualPoint/virtualPoint.h Objects/VirtualPointsHolder/virtualPointsHolder.h)

# Build HLSL shaders
add_custom_target(shaders)

set(HLSL_SHADER_FILES Shader/ps.hlsl Shader/vs.hlsl Shader/hs.hlsl Shader/ds.hlsl)

set_source_files_properties(Shader/vs.hlsl PROPERTIES ShaderType "vs")
set_source_files_properties(Shader/ps.hlsl PROPERTIES ShaderType "ps")
set_source_files_properties(Shader/hs.hlsl PROPERTIES ShaderType "hs")
set_source_files_properties(Shader/ds.hlsl PROPERTIES ShaderType "ds")
set_source_files_properties(${HLSL_SHADER_FILES} PROPERTIES ShaderModel "5_0")

foreach(FILE ${HLSL_SHADER_FILES})
    get_filename_component(FILE_WE ${FILE} NAME_WE)
    get_source_file_property(shadertype ${FILE} ShaderType)
    get_source_file_property(shadermodel ${FILE} ShaderModel)
    add_custom_command(TARGET shaders
            COMMAND fxc.exe /nologo /Emain /T${shadertype}_${shadermodel} /Zi /Fo ${CMAKE_BINARY_DIR}/${FILE_WE}.cso /Fd ${CMAKE_BINARY_DIR}/${FILE_WE}.pdb ${FILE}
            MAIN_DEPENDENCY ${FILE}
            COMMENT "HLSL ${FILE}"
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
            VERBATIM)
endforeach(FILE)

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
target_link_libraries(MG1 PRIVATE "${D3D11_LIBRARY}")
add_dependencies(MG1 shaders)

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
