GUID 		= {5e14164e-1a08-46e2-adf3-f44776b48b1c}
TEMPLATE	= app
TARGET		= customlayout

CONFIG		+= qt warn_on release
DEPENDPATH	= ../../include

QTDIR_build:REQUIRES	= large-config

HEADERS		= border.h \
		  card.h \
		  flow.h
SOURCES		= border.cpp \
		  card.cpp \
		  flow.cpp \
		  main.cpp
