TEMPLATE    = app
CONFIG	    += qt warn_off activeqt
TARGET	    = tetrixax
HEADERS	    = gtetrix.h \
	      qdragapp.h \
	      qtetrix.h \
	      qtetrixb.h \
	      tpiece.h
SOURCES	    = gtetrix.cpp \
	      qdragapp.cpp \
	      qtetrix.cpp \
	      qtetrixb.cpp \
	      tetrix.cpp \
	      tpiece.cpp

RC_FILE	    = ../../control/qaxserver.rc
