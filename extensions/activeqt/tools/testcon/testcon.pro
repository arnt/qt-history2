TEMPLATE = app

CONFIG	+= uic3 qaxserver qaxserver_no_postlink qaxcontainer
QT += qt3support

# ui_qaxselect.h
INCLUDEPATH += ../../container/debug \
    ../../container/release

SOURCES	 = main.cpp docuwindow.cpp
HEADERS	 = docuwindow.h
FORMS	 = mainwindow.ui invokemethod.ui changeproperties.ui ambientproperties.ui controlinfo.ui
RC_FILE	 = testcon.rc

win32-borland {
    QMAKE_POST_LINK = -midl testcon.idl
} else {
    QMAKE_POST_LINK = midl testcon.idl && move testcon.tlb $(TARGETDIR)
}
