TEMPLATE	= app
CONFIG		+= qt warn_on release dll
HEADERS		= 
SOURCES		= main.cpp
win32:LIBS	+= $(QTDIR)\lib\qnp.lib $(QTDIR)\lib\qtmain.lib
unix:LIBS	= -lqnp -lXt
TARGET		= npmovies
DEPENDPATH=../../include
