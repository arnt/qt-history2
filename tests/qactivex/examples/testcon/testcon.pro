TEMPLATE    = app
CONFIG	    += qt warn_off release
SOURCES	    = main.cpp

TARGET	    = testcon
INCLUDEPATH += ..\..\container
LIBS	    += $(QTDIR)\lib\qactivex.lib
