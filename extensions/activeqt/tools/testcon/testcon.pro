REQUIRES    = shared workspace
TEMPLATE    = app
CONFIG	    += qt warn_off

SOURCES	    = main.cpp ../../shared/types.cpp docuwindow.cpp
HEADERS	    = docuwindow.h
FORMS	    = mainwindow.ui invokemethod.ui changeproperties.ui ambientproperties.ui controlinfo.ui 
RC_FILE	    = testcon.rc

LIBS	    += -lqaxcontainer
