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
    TypeInfo.cpp \
    TcpServer.cpp \
    Tcp.cpp \
    SymbolUtils.cpp \
    Serialization.cpp \
    SamplingProfiler.cpp \
    ProcessUtils.cpp \
    Pdb.cpp \
    Params.cpp \
    OrbitUnreal.cpp \
    OrbitType.cpp \
    OrbitThread.cpp \
    OrbitSession.cpp \
    OrbitRule.cpp \
    OrbitProcess.cpp \
    OrbitModule.cpp \
    OrbitFunction.cpp \
    OrbitDia.cpp \
    ModuleManager.cpp \
    MiniDump.cpp \
    MemoryTracker.cpp \
    LogInterface.cpp \
    Injection.cpp \
    FunctionStats.cpp \
    EventUtils.cpp \
    EventTracer.cpp \
    EventCallbacks.cpp \
    EventBuffer.cpp \
    Diff.cpp \
    DiaParser.cpp \
    DiaManager.cpp \
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
    DiaManager.h \
    DiaParser.h \
    Diff.h \
    EventBuffer.h \
    EventCallbacks.h \
    EventClasses.h \
    EventGuid.h \
    EventTracer.h \
    EventUtils.h \
    Hashing.h \
    Injection.h \
    LogInterface.h \
    MemoryTracker.h \
    MiniDump.h \
    ModuleManager.h \
    OrbitDia.h \
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
    SymbolUtils.h \
    Tcp.h \
    TcpServer.h \
    Variable.h \
    VariableTracing.h \
    Version.h
unix {
    target.path = /usr/lib
    INSTALLS += target
}
