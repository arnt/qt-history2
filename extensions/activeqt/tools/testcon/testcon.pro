TEMPLATE = app

CONFIG	+= qt warn_off

QMAKE_LIBS_QT_ENTRY = 
LIBS	+= -lqaxserver -lqaxcontainer

REQUIRES = shared workspace

SOURCES	 = main.cpp docuwindow.cpp
HEADERS	 = docuwindow.h
FORMS	 = mainwindow.ui invokemethod.ui changeproperties.ui ambientproperties.ui controlinfo.ui
RC_FILE	 = testcon.rc

win32-borland {
    QMAKE_POST_LINK = midl testcon.idl
} else {
    QMAKE_POST_LINK = midl testcon.idl && move testcon.tlb $(TARGETDIR)
}
