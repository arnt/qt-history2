REQUIRES    = shared
TEMPLATE    = app
CONFIG	    += qt warn_off

SOURCES	    = main.cpp ../../shared/types.cpp
FORMS	    = mainwindow.ui invokemethod.ui changeproperties.ui ambientproperties.ui controlinfo.ui 

LIBS	    += $$QT_BUILD_TREE/lib/qaxcontainer.lib
