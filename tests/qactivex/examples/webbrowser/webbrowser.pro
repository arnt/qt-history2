TEMPLATE    =app
CONFIG	    += qt warn_on release

SOURCES	    = main.cpp 
FORMS	    = mainwindow.ui 

INCLUDEPATH += ../../container
LIBS	    += $(QTDIR)/lib/qactivex.lib
