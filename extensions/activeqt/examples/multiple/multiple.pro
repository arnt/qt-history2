# Project ID used by some IDEs
GUID 	 = {eba641a0-f85b-4343-981e-3464bb6db413}
TEMPLATE = lib
TARGET	 = multipleax

CONFIG	+= qt warn_off activeqt dll

SOURCES	 = main.cpp
HEADERS	 = ax1.h ax2.h
RC_FILE	 = ../../control/qaxserver.rc
DEF_FILE = ../../control/qaxserver.def
