TEMPLATE	= app
CONFIG+= qt warn_on release
HEADERS		= quuidgen.h
SOURCES		= main.cpp quuidgen.cpp
INTERFACES	= quuidbase.ui
TARGET		= quuidgen
unix:LIBS		+= -L/lib -luuid
DESTDIR		= ../../bin
