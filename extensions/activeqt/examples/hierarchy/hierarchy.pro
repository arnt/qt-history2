TEMPLATE = lib
TARGET	 = hierarchyax

CONFIG	+= qt warn_off qaxserver dll

SOURCES	 = objects.cpp main.cpp
HEADERS	 = objects.h
RC_FILE	 = ../../control/qaxserver.rc
DEF_FILE = ../../control/qaxserver.def
