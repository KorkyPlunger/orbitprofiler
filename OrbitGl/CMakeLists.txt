add_library(OrbitGl STATIC
    shader.c
    mat4.c

    App.cpp
    Batcher.cpp
    BlackBoard.cpp
    CallStackDataView.cpp
    CaptureSerializer.cpp
    CaptureWindow.cpp
    Card.cpp
    DataViewModel.cpp
    Debugger.cpp
    Disassembler.cpp
    EventTrack.cpp
    FunctionDataView.cpp
    GlCanvas.cpp
    GlobalDataView.cpp
    GlPanel.cpp
    GlSlider.cpp
    GlUtils.cpp
    HomeWindow.cpp
    ImGuiOrbit.cpp
    ImmediateWindow.cpp
    LiveFunctionDataView.cpp
    LogDataView.cpp
    ModuleDataView.cpp
    PickingManager.cpp
    PluginCanvas.cpp
    PluginManager.cpp
    ProcessDataView.cpp
    RuleEditor.cpp
    SamplingReport.cpp
    SamplingReportDataView.cpp
    SessionsDataView.cpp
    TextBox.cpp
    TextRenderer.cpp
    ThreadDataViewGl.cpp
    ThreadView.cpp
    TimeGraph.cpp
    TimeGraphLayout.cpp
    TypeDataView.cpp
)

target_include_directories(OrbitGl
PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
PRIVATE
    ${FREETYPE_GL_INCLUDE_DIR}
    ${EXTERN_ROOT}/capstone/include
    ${FREEGLUT_INCLUDE_DIR}
)

target_link_libraries(OrbitGl
    OrbitCore
    OrbitBase

    ${FREEGLUT_LIB}
    ${FREETYPE_LIBRARIES}
    ${IMGUI_LIB}
    ${GLEW_LIBRARIES}
    ${FREETYPE_GL_LIB}

    debug
        ${EXTERN_ROOT}/capstone/msvc/x64/Debug/capstone.lib
    optimized
        ${EXTERN_ROOT}/capstone/msvc/x64/Release/capstone.lib
)
