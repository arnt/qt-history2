TEMPLATE = app

CONFIG	+= qt warn_off
LIBS	+= -lqaxcontainer

REQUIRES = shared workspace

SOURCES	 = main.cpp docuwindow.cpp
HEADERS	 = docuwindow.h
FORMS	 = mainwindow.ui invokemethod.ui changeproperties.ui ambientproperties.ui controlinfo.ui
RC_FILE	 = testcon.rc
