TEMPLATE    = lib
CONFIG	    += qt warn_off dll activeqt
TARGET	    = testcontrol
SOURCES	    = control.cpp

RC_FILE	    = $$QT_SOURCE_TREE/extensions/activeqt/control/qaxserver.rc
DEF_FILE    = $$QT_SOURCE_TREE/extensions/activeqt/control/qaxserver.def
