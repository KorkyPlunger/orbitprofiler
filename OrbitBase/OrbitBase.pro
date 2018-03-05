#-------------------------------------------------
#
# Project created by QtCreator 2018-02-19T22:16:24
#
#-------------------------------------------------

QT       -= core gui

TARGET = OrbitBase
TEMPLATE = lib
CONFIG += staticlib

SOURCES += \
    Utils.cpp \
    TimerManager.cpp \
    TcpEntity.cpp \
    ScopeTimer.cpp \
    Path.cpp \
    OrbitAsio.cpp \
    Message.cpp \
    Log.cpp \
    CrashHandler.cpp \
    CallstackPOD.cpp \
    ../external/xxHash-r42/xxhash.c \
    ../external/xxHash-r42/xxhsum.c

HEADERS += \
    BaseTypes.h \
    Callstack.h \
    CallstackTypes.h \
    Context.h \
    CrashHandler.h \
    FunctionArgs.h \
    FunctionStats.h \
    Log.h \
    Message.h \
    OrbitAsio.h \
    OrbitDbgHelp.h \
    Path.h \
    PrintVar.h \
    Profiling.h \
    ScopeTimer.h \
    SerializationMacros.h \
    TcpEntity.h \
    TcpForward.h \
    Threading.h \
    TimerManager.h \
    TypeInfoStructs.h \
    Utils.h \
    OrbitTypes.h \
    ../external/xxHash-r42/xxhash.h

INCLUDEPATH += \
    ../external \
    ../external/concurrentqueue \
    ../external/multicore/common \
    ../external/stlsoft-1.9.118/include \
    ../external/oqpi/include \
    ../external/asio/include \
    ../external/breakpad/src

config_dir = release
CONFIG( debug, debug|release ){
    config_dir = debug
}

OBJECTS_DIR = $$PWD/../intermediate/x64/OrbitBase/$$config_dir/
DESTDIR     = $$PWD/../intermediate/x64/OrbitBase/$$config_dir/
UI_DIR      = $$PWD/../GeneratedFiles/OrbitBase/$$config_dir/
MOC_DIR     = $$PWD/../GeneratedFiles/OrbitBase/$$config_dir/

unix {
    target.path = /usr/lib
    INSTALLS += target
}
