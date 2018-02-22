#-------------------------------------------------
#
# Project created by QtCreator 2018-02-19T22:02:53
#
#-------------------------------------------------

QT       -= core gui

TARGET = OrbitCoreQt
TEMPLATE = lib
CONFIG += staticlib

SOURCES += orbitcoreqt.cpp \
    Version.cpp \
    VariableTracing.cpp \
    Variable.cpp \
    TcpServer.cpp \
    Tcp.cpp \
    Serialization.cpp \
    SamplingProfiler.cpp \
    ProcessUtils.cpp \
    Params.cpp \
    OrbitUnreal.cpp \
    OrbitType.cpp \
    OrbitThread.cpp \
    OrbitSession.cpp \
    OrbitRule.cpp \
    OrbitProcess.cpp \
    OrbitModule.cpp \
    OrbitFunction.cpp \
    ModuleManager.cpp \
    MiniDump.cpp \
    MemoryTracker.cpp \
    LogInterface.cpp \
    Injection.cpp \
    FunctionStats.cpp \
    EventBuffer.cpp \
    CoreApp.cpp \
    ContextSwitch.cpp \
    Capture.cpp \
    Callstack.cpp

HEADERS += orbitcoreqt.h \
    BlockChain.h \
    Capture.h \
    ContextSwitch.h \
    CoreApp.h \
    cvconst.h \
    EventBuffer.h \
    Hashing.h \
    Injection.h \
    LogInterface.h \
    MemoryTracker.h \
    MiniDump.h \
    ModuleManager.h \
    OrbitFunction.h \
    OrbitModule.h \
    OrbitProcess.h \
    OrbitRule.h \
    OrbitSession.h \
    OrbitThread.h \
    OrbitType.h \
    OrbitUnreal.h \
    Params.h \
    Pdb.h \
    ProcessUtils.h \
    RingBuffer.h \
    SamplingProfiler.h \
    Serialization.h \
    ServerTimerManager.h \
    Tcp.h \
    TcpServer.h \
    Variable.h \
    VariableTracing.h \
    Version.h

INCLUDEPATH += \
    ../OrbitBase \
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
    ../external/cereal-1.1.2/include

unix {
    target.path = /usr/lib
    INSTALLS += target
}
