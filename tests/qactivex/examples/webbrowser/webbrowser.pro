TEMPLATE    =app
CONFIG	    += qt warn_on

SOURCES	    = main.cpp 
FORMS	    = mainwindow.ui 

INCLUDEPATH += ../../container
LIBS	    += $(QTDIR)/lib/qaxwidget.lib
