TEMPLATE    = app
CONFIG	    += qt warn_off activeqt
TARGET	    = tetrax
HEADERS	    = gtetrax.h \
	      qdragapp.h \
	      qtetrax.h \
	      qtetraxb.h \
	      tpiece.h
SOURCES	    = gtetrax.cpp \
	      qdragapp.cpp \
	      qtetrax.cpp \
	      qtetraxb.cpp \
	      tetrax.cpp \
	      tpiece.cpp

RC_FILE	    = ../../control/qaxserver.rc
