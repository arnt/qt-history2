# Project ID used by some IDEs
GUID 	 = {1ccbf5cf-42ef-4cbb-b120-aa109d9a9632}
TEMPLATE = app
TARGET	 = tetrax

CONFIG	+= qt warn_off activeqt

HEADERS	 = gtetrax.h \
	   qdragapp.h \
	   qtetrax.h \
	   qtetraxb.h \
	   tpiece.h
SOURCES	 = gtetrax.cpp \
	   qdragapp.cpp \
	   qtetrax.cpp \
	   qtetraxb.cpp \
	   tetrax.cpp \
	   tpiece.cpp

RC_FILE	 = ../../control/qaxserver.rc
