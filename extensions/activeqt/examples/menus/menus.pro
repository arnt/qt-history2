TEMPLATE    = app
CONFIG	    += qt warn_off activeqt
TARGET	    = menusax
SOURCES	    = main.cpp menus.cpp
HEADERS	    = menus.h

RC_FILE	    = ../../control/qaxserver.rc
INCLUDEPATH += ../../control
