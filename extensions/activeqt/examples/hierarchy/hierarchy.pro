GUID 	 = {5147ece1-3d65-490d-9954-043a11de24a3}
TEMPLATE = lib
TARGET	 = hierarchyax

CONFIG	+= qt warn_off activeqt dll

SOURCES	 = objects.cpp main.cpp
HEADERS	 = objects.h
RC_FILE	 = ../../control/qaxserver.rc
DEF_FILE = ../../control/qaxserver.def
