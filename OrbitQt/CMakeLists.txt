set(QTDIR "C:/Qt/5.8/msvc2015_64")
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${QTDIR}")

find_package(Qt5 CONFIG REQUIRED 
	HINTS "${QTDIR}" 
	COMPONENTS Widgets Core
)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

add_executable(OrbitQt
    licensedialog.cpp
    main.cpp
    orbitcodeeditor.cpp
    orbitdataviewpanel.cpp
    orbitdiffdialog.cpp
    orbitdisassemblydialog.cpp
    orbitglwidget.cpp
    orbitglwidgetwithheader.cpp
    orbitmainwindow.cpp
    orbitsamplingreport.cpp
    orbittablemodel.cpp
    orbittreeitem.cpp
    orbittreemodel.cpp
    orbittreeview.cpp
    orbitvisualizer.cpp
    orbitwatchwidget.cpp
    outputdialog.cpp
    processlauncherwidget.cpp
    showincludesdialog.cpp

    qtpropertybrowser/qtbuttonpropertybrowser.cpp
    qtpropertybrowser/qteditorfactory.cpp
    qtpropertybrowser/qtgroupboxpropertybrowser.cpp
    qtpropertybrowser/qtpropertybrowser.cpp
    qtpropertybrowser/qtpropertybrowserutils.cpp
    qtpropertybrowser/qtpropertymanager.cpp
    qtpropertybrowser/qttreepropertybrowser.cpp
    qtpropertybrowser/qtvariantproperty.cpp
)

if(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
	set_target_properties(OrbitQt PROPERTIES COMPILE_FLAGS /wd4127)
endif()

target_link_libraries (OrbitQt
	OrbitGl
	OrbitCore
	OrbitBase
	Qt5::Widgets
	Qt5::Core
	${CURL_LIBRARIES}
)
