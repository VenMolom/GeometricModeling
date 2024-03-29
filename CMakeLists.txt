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
        mainwindow.cpp
        mainwindow.h
        mainwindow.ui
        scene.cpp scene.h
        Objects/object.cpp
        Objects/object.h
        camera.cpp camera.h
        Controls/controls.h
        Controls/controls.cpp
        Controls/controls.ui
        DirectX/DXRenderer/dxRenderer.cpp
        DirectX/DXRenderer/dxRenderer.h
        DirectX/DXDevice/dxDevice.cpp
        DirectX/DXDevice/dxDevice.h
        DirectX/DXDevice/dxptr.h
        DirectX/DXStructures/dxStructures.cpp
        DirectX/DXStructures/dxStructures.h
        DirectX/DXDevice/exceptions.cpp
        DirectX/DXDevice/exceptions.h
        DirectX/renderer.h
        Objects/Parametric/parametricObject.h
        Objects/Parametric/torus.cpp
        Objects/Parametric/torus.h
        Resources.qrc Handlers/inputHandler.cpp
        Handlers/inputHandler.h
        Objects/Cursor/cursor.cpp
        Objects/Cursor/cursor.h
        Utils/utils3D.h
        Utils/utils3D.cpp
        Objects/Point/point.cpp
        Objects/Point/point.h
        ObjectListItem/objectListItem.cpp
        ObjectListItem/objectListItem.h
        Objects/CompositeObject/compositeObject.cpp
        Objects/CompositeObject/compositeObject.h
        Objects/Curve/brezierC0.cpp
        Objects/Curve/brezierC0.h
        Objects/Curve/brezierCurve.cpp
        Objects/Curve/brezierCurve.h
        Objects/Curve/brezierC2.cpp
        Objects/Curve/brezierC2.h
        Controls/Modules/ObjectModule/objectmodule.h
        Controls/Modules/ObjectModule/objectmodule.cpp
        Controls/Modules/ObjectModule/objectmodule.ui
        Controls/Modules/ParametricModule/parametricmodule.h
        Controls/Modules/ParametricModule/parametricmodule.cpp
        Controls/Modules/ParametricModule/parametricmodule.ui
        Controls/Modules/TorusModule/torusmodule.h
        Controls/Modules/TorusModule/torusmodule.cpp
        Controls/Modules/TorusModule/torusmodule.ui
        Controls/Modules/CurveModule/curvemodule.h
        Controls/Modules/CurveModule/curvemodule.cpp
        Controls/Modules/CurveModule/curvemodule.ui
        Controls/Modules/MoveModule/movemodule.h
        Controls/Modules/MoveModule/movemodule.cpp
        Controls/Modules/MoveModule/movemodule.ui
        Controls/Modules/TransformModule/transformmodule.h
        Controls/Modules/TransformModule/transformmodule.cpp
        Controls/Modules/TransformModule/transformmodule.ui
        Controls/Modules/ScreenMoveModule/screenmovemodule.h
        Controls/Modules/ScreenMoveModule/screenmovemodule.cpp
        Controls/Modules/ScreenMoveModule/screenmovemodule.ui
        Controls/Modules/BSplineModule/bsplinemodule.h
        Controls/Modules/BSplineModule/bsplinemodule.cpp
        Controls/Modules/BSplineModule/bsplinemodule.ui
        Objects/Point/virtualPoint.cpp
        Objects/Point/virtualPoint.h
        Objects/Point/virtualPointsHolder.h
        Objects/Grid/grid.cpp
        Objects/Grid/grid.h
        Objects/renderable.cpp
        Objects/renderable.h
        Objects/linestrip.cpp
        Objects/linestrip.h
        Objects/Curve/curve.cpp
        Objects/Curve/curve.h
        Objects/Curve/interpolationCurveC2.cpp
        Objects/Curve/interpolationCurveC2.h
        Objects/objectFactory.cpp
        Objects/objectFactory.h
        Handlers/screenTransform.cpp
        Handlers/screenTransform.h
        Controls/scenecontrols.h
        Controls/scenecontrols.cpp
        Controls/scenecontrols.ui
        DirectX/stereoscopicRenderer.h
        DirectX/DXDevice/ptr_vector.h
        DirectX/DXDevice/compressed_pair.h
        Objects/Patch/bicubicC0.cpp Objects/Patch/bicubicC0.h
        Objects/Patch/patch.cpp Objects/Patch/patch.h
        Objects/Patch/bicubicC0Creator.cpp
        Objects/Patch/bicubicC0Creator.h
        Objects/creator.h
        Controls/Modules/PatchCreatorModule/patchcreatormodule.h
        Controls/Modules/PatchCreatorModule/patchcreatormodule.cpp
        Controls/Modules/PatchCreatorModule/patchcreatormodule.ui
        Objects/Patch/patchCreator.h
        Objects/linelist.cpp Objects/linelist.h
        Controls/Modules/PatchModule/patchmodule.h
        Controls/Modules/PatchModule/patchmodule.cpp
        Controls/Modules/PatchModule/patchmodule.ui
        Objects/Point/composableVirtualPoint.cpp
        Objects/Point/composableVirtualPoint.h
        Objects/Patch/bicubicC2.cpp
        Objects/Patch/bicubicC2.h
        Objects/Patch/bicubicC2Creator.cpp
        Objects/Patch/bicubicC2Creator.h
        Utils/gregoryUtils.cpp Utils/gregoryUtils.h
        Objects/Patch/gregoryPatch.cpp Objects/Patch/gregoryPatch.h
        Controls/intersectdialog.h
        Controls/intersectdialog.cpp
        Controls/intersectdialog.ui
        Handlers/intersectHandler.cpp
        Handlers/intersectHandler.h
        Objects/Intersection/intersection.cpp
        Objects/Intersection/intersection.h
        Objects/Intersection/intersectionInstance.cpp
        Objects/Intersection/intersectionInstance.h
        Controls/Modules/IntersectionInstanceModule/intersectioninstancemodule.h
        Controls/Modules/IntersectionInstanceModule/intersectioninstancemodule.cpp
        Controls/Modules/IntersectionInstanceModule/intersectioninstancemodule.ui
        Controls/intersectiontexturedialog.h
        Controls/intersectiontexturedialog.cpp
        Controls/intersectiontexturedialog.ui
        Controls/Modules/IntersectionModule/intersectionmodule.h
        Controls/Modules/IntersectionModule/intersectionmodule.cpp
        Controls/Modules/IntersectionModule/intersectionmodule.ui
        Utils/fileParser.cpp Utils/fileParser.h
        Controls/Modules/CNCRouterModule/cncroutermodule.h
        Controls/Modules/CNCRouterModule/cncroutermodule.cpp
        Controls/Modules/CNCRouterModule/cncroutermodule.ui
        Objects/updatable.h
        Objects/CNC/CNCRouter.cpp
        Objects/CNC/CNCRouter.h
        Objects/CNC/CNCTool.cpp
        Objects/CNC/CNCTool.h
        Objects/CNC/mesh.cpp
        Objects/CNC/mesh.h
        Utils/DDSTextureLoader.cpp
        Utils/DDSTextureLoader.h
        Utils/WICTextureLoader.cpp
        Utils/WICTextureLoader.h
        Utils/pathsCreator.cpp
        Utils/pathsCreator.h Utils/pathsCreatorHelper.cpp Utils/pathsCreatorHelper.h Objects/Parametric/toolDistantSurface.cpp Objects/Parametric/toolDistantSurface.h)

# Build HLSL shaders
add_custom_target(shaders)

set(HLSL_SHADER_FILES
        Shader/ps.hlsl
        Shader/vs.hlsl
        Shader/hsBrezier.hlsl
        Shader/hsBicubic.hlsl
        Shader/hsGregory.hlsl
        Shader/dsBrezier.hlsl
        Shader/dsBicubic.hlsl
        Shader/dsGregory.hlsl
        Shader/dsBicubicDeBoor.hlsl
        Shader/psCameraFade.hlsl
        Shader/vsBillboard.hlsl
        Shader/vsNoProjection.hlsl
        Shader/gsPoint.hlsl
        Shader/vsStereo.hlsl
        Shader/psStereo.hlsl
        Shader/vsSelector.hlsl
        Shader/psSelector.hlsl
        Shader/vsTexture.hlsl
        Shader/psTexture.hlsl
        Shader/vsParam.hlsl
        Shader/psParam.hlsl
        Shader/vsPhong.hlsl
        Shader/vsPhongTex.hlsl
        Shader/psPhong.hlsl
        Shader/psPhongTex.hlsl
        Shader/csNormal.hlsl
        Shader/csHeight.hlsl)

set_source_files_properties(Shader/vs.hlsl PROPERTIES ShaderType "vs")
set_source_files_properties(Shader/vsBillboard.hlsl PROPERTIES ShaderType "vs")
set_source_files_properties(Shader/vsNoProjection.hlsl PROPERTIES ShaderType "vs")
set_source_files_properties(Shader/vsStereo.hlsl PROPERTIES ShaderType "vs")
set_source_files_properties(Shader/vsSelector.hlsl PROPERTIES ShaderType "vs")
set_source_files_properties(Shader/vsTexture.hlsl PROPERTIES ShaderType "vs")
set_source_files_properties(Shader/vsParam.hlsl PROPERTIES ShaderType "vs")
set_source_files_properties(Shader/vsPhong.hlsl PROPERTIES ShaderType "vs")
set_source_files_properties(Shader/vsPhongTex.hlsl PROPERTIES ShaderType "vs")
set_source_files_properties(Shader/ps.hlsl PROPERTIES ShaderType "ps")
set_source_files_properties(Shader/psCameraFade.hlsl PROPERTIES ShaderType "ps")
set_source_files_properties(Shader/psStereo.hlsl PROPERTIES ShaderType "ps")
set_source_files_properties(Shader/psSelector.hlsl PROPERTIES ShaderType "ps")
set_source_files_properties(Shader/psTexture.hlsl PROPERTIES ShaderType "ps")
set_source_files_properties(Shader/psParam.hlsl PROPERTIES ShaderType "ps")
set_source_files_properties(Shader/psPhong.hlsl PROPERTIES ShaderType "ps")
set_source_files_properties(Shader/psPhongTex.hlsl PROPERTIES ShaderType "ps")
set_source_files_properties(Shader/hsBrezier.hlsl PROPERTIES ShaderType "hs")
set_source_files_properties(Shader/hsBicubic.hlsl PROPERTIES ShaderType "hs")
set_source_files_properties(Shader/hsGregory.hlsl PROPERTIES ShaderType "hs")
set_source_files_properties(Shader/dsBrezier.hlsl PROPERTIES ShaderType "ds")
set_source_files_properties(Shader/dsBicubic.hlsl PROPERTIES ShaderType "ds")
set_source_files_properties(Shader/dsGregory.hlsl PROPERTIES ShaderType "ds")
set_source_files_properties(Shader/dsBicubicDeBoor.hlsl PROPERTIES ShaderType "ds")
set_source_files_properties(Shader/gsPoint.hlsl PROPERTIES ShaderType "gs")
set_source_files_properties(Shader/csNormal.hlsl PROPERTIES ShaderType "cs")
set_source_files_properties(Shader/csHeight.hlsl PROPERTIES ShaderType "cs")
set_source_files_properties(${HLSL_SHADER_FILES} PROPERTIES ShaderModel "5_0")

foreach (FILE ${HLSL_SHADER_FILES})
    get_filename_component(FILE_WE ${FILE} NAME_WE)
    get_source_file_property(shadertype ${FILE} ShaderType)
    get_source_file_property(shadermodel ${FILE} ShaderModel)
    add_custom_command(TARGET shaders
            COMMAND fxc.exe /nologo /Emain /T${shadertype}_${shadermodel} /Zi /Fo ${CMAKE_BINARY_DIR}/${FILE_WE}.cso /Fd ${CMAKE_BINARY_DIR}/${FILE_WE}.pdb ${FILE}
            MAIN_DEPENDENCY ${FILE}
            COMMENT "HLSL ${FILE}"
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
            VERBATIM)
endforeach (FILE)

if (${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(MG1
            MANUAL_FINALIZATION
            ${PROJECT_SOURCES}
            )
    # Define target properties for Android with Qt 6 as:
    #    set_property(TARGET MG1 APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
    #                 ${CMAKE_CURRENT_SOURCE_DIR}/android)
    # For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation
else ()
    if (ANDROID)
        add_library(MG1 SHARED
                ${PROJECT_SOURCES}
                )
        # Define properties for Android with Qt 5 after find_package() calls as:
        #    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
    else ()
        add_executable(MG1
                ${PROJECT_SOURCES}
                )
    endif ()
endif ()

add_subdirectory(Scene-Serializer/cpp/Serializer)
target_link_libraries(MG1 PRIVATE Scene-Serializer/cpp/Serializer/MGSerializer)
target_include_directories(MG1 PRIVATE Scene-Serializer/cpp/Serializer/Serializer)

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

if (QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(MG1)
endif ()
