#-------------------------------------------------
#
# Project created by QtCreator 2018-02-21T23:11:29
#
#-------------------------------------------------

QT       -= core gui

TARGET = OrbitGl
TEMPLATE = lib
CONFIG += staticlib

SOURCES += \
    App.cpp \
    Batcher.cpp \
    BlackBoard.cpp \
    CallStackDataView.cpp \
    CaptureSerializer.cpp \
    CaptureWindow.cpp \
    Card.cpp \
    DataViewModel.cpp \
    EventTrack.cpp \
    FunctionDataView.cpp \
    GlCanvas.cpp \
    GlobalDataView.cpp \
    GlPanel.cpp \
    GlSlider.cpp \
    GlUtils.cpp \
    HomeWindow.cpp \
    ImGuiOrbit.cpp \
    ImmediateWindow.cpp \
    LiveFunctionDataView.cpp \
    LogDataView.cpp \
    ModuleDataView.cpp \
    PickingManager.cpp \
    PluginCanvas.cpp \
    PluginManager.cpp \
    ProcessDataView.cpp \
    RuleEditor.cpp \
    SamplingReport.cpp \
    SamplingReportDataView.cpp \
    SessionsDataView.cpp \
    TextBox.cpp \
    TextRenderer.cpp \
    ThreadDataViewGl.cpp \
    ThreadView.cpp \
    TimeGraph.cpp \
    TimeGraphLayout.cpp \
    TypeDataView.cpp \
    mat4.c \
    shader.c

HEADERS += \
    App.h \
    Batcher.h \
    BlackBoard.h \
    CallStackDataView.h \
    CaptureSerializer.h \
    CaptureWindow.h \
    Card.h \
    CoreMath.h \
    DataViewModel.h \
    DataViewTypes.h \
    Debugger.h \
    EventTrack.h \
    FunctionDataView.h \
    Geometry.h \
    GlCanvas.h \
    GlobalDataView.h \
    GlPanel.h \
    GlSlider.h \
    GlUtils.h \
    HomeWindow.h \
    Images.h \
    ImGuiOrbit.h \
    ImmediateWindow.h \
    LiveFunctionDataView.h \
    LogDataView.h \
    mat4.h \
    ModuleDataView.h \
    OpenGl.h \
    PickingManager.h \
    PluginCanvas.h \
    PluginManager.h \
    ProcessDataView.h \
    resource.h \
    RuleEditor.h \
    SamplingReport.h \
    SamplingReportDataView.h \
    SessionsDataView.h \
    shader.h \
    TextBox.h \
    TextRenderer.h \
    ThreadDataViewGl.h \
    ThreadView.h \
    TimeGraph.h \
    TimeGraphLayout.h \
    TypeDataView.h

INCLUDEPATH += \
    ../OrbitBase \
    ../OrbitCore \
    ../external \
    ../external/xxHash-r42 \
    ../external/concurrentqueue \
    ../external/multicore/common \
    ../external/stlsoft-1.9.118/include \
    ../external/oqpi/include \
    ../external/asio/include \
    ../external/breakpad/src \
    ../external/curl-7.52.1/include \
    ../external/websocketpp \
    ../external/cereal-1.1.2/include \
    ../external/freetype-gl \
    ../external/glew-2.0.0/include \
    ../external/freeglut-2.8.1/include \
    ../external/imgui \

unix {
    target.path = /usr/lib
    INSTALLS += target
}
