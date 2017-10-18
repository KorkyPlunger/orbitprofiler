#find_package(Qt REQUIRED)
#find_package(Qt5Widgets REQUIRED)

#set(CMAKE_AUTOMOC ON)
#set(CMAKE_AUTOUIC ON)
#set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(QTDIR "C:/Qt/5.8/msvc2015_64")
include_directories(
	${QTDIR}/include
	${QTDIR}/include
	${QTDIR}/include/QtWidgets
	${QTDIR}/include/QtGui
	${QTDIR}/include/QtANGLE
	${QTDIR}/include/QtCore
	${QTDIR}/mkspecs/win32-msvc2015
)
#licensedialog.ui
#orbitdataviewpanel.ui
#orbitdiffdialog.ui
#orbitdisassemblydialog.ui
#orbitglwidgetwithheader.ui
#orbitmainwindow.ui
#orbitsamplingreport.ui
#orbitvisualizer.ui
#orbitwatchwidget.ui
#outputdialog.ui
#processlauncherwidget.ui
#showincludesdialog.ui

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
)

#target_link_libraries (OrbitQt Qt5::Widgets)
target_link_libraries(OrbitQt OrbitGl OrbitCore OrbitBase)
