TEMPLATE = app

CONFIG	+= uic3
QT += compat

QMAKE_LIBS_QT_ENTRY = 
LIBS	+= -lqaxserver -lqaxcontainer

SOURCES	 = main.cpp docuwindow.cpp
HEADERS	 = docuwindow.h ../../container/qaxselect.h
FORMS	 = mainwindow.ui invokemethod.ui changeproperties.ui ambientproperties.ui controlinfo.ui
RC_FILE	 = testcon.rc

win32-borland {
    QMAKE_POST_LINK = -midl testcon.idl
} else {
    QMAKE_POST_LINK = midl testcon.idl && move testcon.tlb $(TARGETDIR)
}
