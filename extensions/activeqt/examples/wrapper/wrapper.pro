GUID 	 = {178f3b0f-6f9a-4d1a-8e3f-8f198ecbbc11}
TEMPLATE = lib
TARGET	 = wrapperax

CONFIG	+= qt warn_off activeqt dll

SOURCES	 = main.cpp
RC_FILE	 = ../../control/qaxserver.rc
DEF_FILE = ../../control/qaxserver.def
