TEMPLATE = lib
TARGET	 = wrapperax

CONFIG	+= qt warn_off qaxserver dll

SOURCES	 = main.cpp
RC_FILE	 = ../../../extensions/activeqt/control/qaxserver.rc
DEF_FILE = ../../../extensions/activeqt/control/qaxserver.def
