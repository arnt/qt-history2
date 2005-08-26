TEMPLATE = app

CONFIG	+= qaxserver qaxserver_no_postlink qaxcontainer
# QT += qt3support

# ui_qaxselect.h
INCLUDEPATH += ../../container/debug \
    ../../container/release

SOURCES	 = main.cpp docuwindow.cpp mainwindow.cpp invokemethod.cpp changeproperties.cpp ambientproperties.cpp controlinfo.cpp
HEADERS	 = docuwindow.h mainwindow.h invokemethod.h changeproperties.h ambientproperties.h controlinfo.h
FORMS	 = mainwindow.ui invokemethod.ui changeproperties.ui ambientproperties.ui controlinfo.ui
RC_FILE	 = testcon.rc

win32-borland {
    QMAKE_POST_LINK = -midl testcon.idl
} else {
    !win32-g++:QMAKE_POST_LINK = midl testcon.idl && move testcon.tlb $(TARGETDIR)
}
