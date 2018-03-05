#-------------------------------------------------
#
# Project created by QtCreator 2016-09-15T10:33:33
#
#-------------------------------------------------

QT += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = orbit
TEMPLATE = app

DEFINES += \
    ORBIT_PROFILER \
    UNICODE

SOURCES += \
    main.cpp \
    licensedialog.cpp \
    orbitcodeeditor.cpp \
    orbitdataviewpanel.cpp \
    orbitglwidget.cpp \
    orbitglwidgetwithheader.cpp \
    orbitmainwindow.cpp \
    orbitsamplingreport.cpp \
    orbittablemodel.cpp \
    orbittreeview.cpp \
    outputdialog.cpp \
    processlauncherwidget.cpp \
    showincludesdialog.cpp \
    orbittreeitem.cpp \
    orbittreemodel.cpp \
    orbitdiffdialog.cpp \
    orbitdisassemblydialog.cpp \
    orbitvisualizer.cpp

HEADERS += \
    orbitmainwindow.h \
    orbitglwidget.h \
    orbittreeview.h \
    orbittablemodel.h \
    orbitdataviewpanel.h \
    orbitsamplingreport.h \
    orbitglwidgetwithheader.h \
    orbitcodeeditor.h \
    licensedialog.h \
    outputdialog.h \
    processlauncherwidget.h \
    showincludesdialog.h \
    orbittreeitem.h \
    orbittreemodel.h \
    orbitdiffdialog.h \
    orbitdisassemblydialog.h \
    orbitvisualizer.h

FORMS += \
    orbitmainwindow.ui \
    orbitdataviewpanel.ui \
    orbitsamplingreport.ui \
    orbitglwidgetwithheader.ui \
    licensedialog.ui \
    outputdialog.ui \
    processlauncherwidget.ui \
    showincludesdialog.ui \
    orbitdiffdialog.ui \
    orbitwatchwidget.ui \
    orbitdisassemblydialog.ui \
    orbitvisualizer.ui

INCLUDEPATH += \
    ../OrbitBase \
    ../OrbitCore \
    ../OrbitGl \
    ../external \
    ../external/xxHash-r42 \
    ../external/concurrentqueue \
    ../external/multicore/common \
    ../external/stlsoft-1.9.118/include \
    ../external/oqpi/include \
    ../external/asio/include \
    ../external/breakpad/src \
    #../external/curl-7.52.1/include \
    ../external/websocketpp \
    ../external/cereal-1.1.2/include \
    ../external/freetype-gl \
    #../external/glew-2.0.0/include \
    #../external/freeglut-2.8.1/include \
    ../external/imgui \
    ../external/qt \

RC_FILE  += \
    OrbitQt.rc

DISTFILES += \
    orbit_16_32_48_256.ico \

CONFIG( debug, debug|release ){
    config_dir = debug
} else {
    config_dir = release
}

OBJECTS_DIR = $$PWD/../intermediate/x64/OrbitQt/$$config_dir/
DESTDIR     = $$PWD/../bin/x64/$$config_dir/
UI_DIR      = $$PWD/../GeneratedFiles/OrbitQt/$$config_dir/
MOC_DIR     = $$PWD/../GeneratedFiles/OrbitQt/$$config_dir/

LIBS += -L$$PWD/../intermediate/x64/OrbitGl/$$config_dir/   -l:libOrbitGl.a
LIBS += -L$$PWD/../intermediate/x64/OrbitCore/$$config_dir/ -l:libOrbitCore.a
LIBS += -L$$PWD/../intermediate/x64/OrbitBase/$$config_dir/ -l:libOrbitBase.a

LIBS += -L$$PWD/../external/freetype-gl/build/ -l:libfreetype-gl.a
LIBS += -L/usr/lib64/                          -lGLEW
LIBS += -lcurl -lglut -lGLU -lfreetype

PRE_TARGETDEPS += $$PWD/../intermediate/x64/OrbitBase/$$config_dir/libOrbitBase.a
PRE_TARGETDEPS += $$PWD/../intermediate/x64/OrbitCore/$$config_dir/libOrbitCore.a
PRE_TARGETDEPS += $$PWD/../intermediate/x64/OrbitGl/$$config_dir/libOrbitGl.a

#SUBDIRS += \
#    ../OrbitBase/OrbitBase.pro
