TEMPLATE	= app
TARGET		= tetrix

CONFIG		+= qt warn_on release
QCONFIG         += compat
DEPENDPATH	= ../../include

QTDIR_build:REQUIRES	= small-config

HEADERS		= gtetrix.h \
		  qdragapp.h \
		  qtetrix.h \
		  qtetrixb.h \
		  tpiece.h
SOURCES		= gtetrix.cpp \
		  qdragapp.cpp \
		  qtetrix.cpp \
		  qtetrixb.cpp \
		  tetrix.cpp \
		  tpiece.cpp
