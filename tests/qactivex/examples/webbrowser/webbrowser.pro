TEMPLATE    = app
CONFIG	    += qt warn_off

SOURCES	    = main.cpp 
FORMS	    = mainwindow.ui 

INCLUDEPATH += ../../container
LIBS	    += $(QTDIR)/lib/qaxcontainer.lib
