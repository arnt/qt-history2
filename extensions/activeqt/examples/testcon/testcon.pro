REQUIRES    = shared
TEMPLATE    = app
CONFIG	    += qt warn_off

SOURCES	    = main.cpp 
FORMS	    = mainwindow.ui invokemethod.ui changeproperties.ui ambientproperties.ui controlinfo.ui 

LIBS	    += qaxcontainer.lib
INCLUDEPATH += ../../container
