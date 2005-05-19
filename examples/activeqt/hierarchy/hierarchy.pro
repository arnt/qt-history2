TEMPLATE = lib
TARGET	 = hierarchyax

CONFIG	+= qt warn_off qaxserver dll

SOURCES	 = objects.cpp main.cpp
HEADERS	 = objects.h
RC_FILE	 = ../../../extensions/activeqt/control/qaxserver.rc
DEF_FILE = ../../../extensions/activeqt/control/qaxserver.def
