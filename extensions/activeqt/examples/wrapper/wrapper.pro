TEMPLATE = lib
TARGET	 = wrapperax

CONFIG	+= qt warn_off qaxserver dll

SOURCES	 = main.cpp
RC_FILE	 = ../../control/qaxserver.rc
DEF_FILE = ../../control/qaxserver.def
