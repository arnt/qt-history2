REQUIRES    = shared workspace
TEMPLATE    = app
CONFIG	    += qt warn_off

SOURCES	    = main.cpp ../../shared/types.cpp
FORMS	    = mainwindow.ui invokemethod.ui changeproperties.ui ambientproperties.ui controlinfo.ui 
RC_FILE	    = testcon.rc

LIBS	    += qaxcontainer.lib
