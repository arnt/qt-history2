REQUIRES    = shared
TEMPLATE    = app
CONFIG	    += qt warn_off

SOURCES	    = main.cpp 
FORMS	    = mainwindow.ui 

INCLUDEPATH += ../../container
LIBS	    += $$QT_BUILD_TREE/lib/qaxcontainer.lib
